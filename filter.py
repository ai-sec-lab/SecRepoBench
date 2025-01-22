from pathlib import Path
import json
import requests
import itertools
from requests.exceptions import RequestException
from base64 import b64encode
from time import sleep
import lizard
from unidiff import PatchSet
from unidiff.errors import UnidiffParseError
from alive_progress import alive_bar
from git import Repo
from pydriller import Repository, Git, ModificationType
import tempfile
import csv
from concurrent.futures import ProcessPoolExecutor, as_completed
import multiprocessing
import pandas as pd
from collections import defaultdict
import time
import re
import os
from projects import *
from collections import defaultdict


def create_proj_samples(samples_each_step, id_2_proj):
    existing_df = None

    for sample_each_step in samples_each_step[1:]:
        project_name_cases = defaultdict(int)
        for stem in sample_each_step['sample ids present']:
            project_name_cases[id_2_proj[stem].lower()] += 1
        ranked_projects = sorted(project_name_cases.items(), key=lambda x: x[1], reverse=True)

        projects = [x[0] for x in ranked_projects]
        num_commits = [x[1] for x in ranked_projects]

        new_df = pd.DataFrame([num_commits], columns=projects, index=[sample_each_step['description']])

        if existing_df is not None:
            # add missing columns to df
            new_cols = {col: 0 for col in existing_df.columns if col not in new_df.columns}
            if new_cols:
                new_cols_df = pd.DataFrame(new_cols, index=new_df.index)
                new_df = pd.concat([new_df, new_cols_df], axis=1)

            # add df to existing_df
            new_df = new_df[existing_df.columns]
            existing_df = pd.concat([existing_df, new_df])
        
        else:
            existing_df = new_df

    return existing_df


def is_cxx_src(path):
    path = path.lower()
    for f in ["fuzz", "test"]:
        if f in path:
            return False
    if any(ext == path.split(".")[-1] for ext in ["c", "cc", "cpp", "cxx", "h", "hh", "hpp", "hxx"]):
        return True
    return False


def is_nontrivial_change(file):
    # remove trivial changes
    src_before = file.source_code_before.split('\n')
    src_after = file.source_code.split('\n')
    diff_parsed = diff_rm_trivial_changes(file.diff_parsed, src_before, src_after)

    # remove multi-line comments from diffs
    if len(diff_parsed['added'] ) > 0 or len(diff_parsed['deleted']) > 0:
        return True
        
    return False


def rm_multi_line_comments(diff_parsed, src):
    # get ranges of multi line comments
    multi_line_comments = []

    in_multi_line_comment = False

    for i, line in enumerate(src):
        line = line.strip()

        if in_multi_line_comment and line.endswith("*/"):
            # multi line comment ended -- ends with */
            end_multline_comment = i + 1
            multi_line_comments.append((start_multline_comment, end_multline_comment))
            in_multi_line_comment = False

        elif not in_multi_line_comment and line.startswith("/*") and not line.endswith("*/"):
                # a new multi line comment has started
                in_multi_line_comment = True
                start_multline_comment = i + 1

    # remove lines in diff_parsed that fall within these ranges
    rm_lines = []
    for diff in diff_parsed:
        for start_multline_comment, end_multline_comment in multi_line_comments:
            if start_multline_comment <= diff[0] <= end_multline_comment:
                rm_lines.append(diff[0])
    
    diff_filter = [diff for diff in diff_parsed if diff[0] not in rm_lines]

    return diff_filter


def rm_single_line_comments(diff_parsed):
    diff_filtered = []
    for diff in diff_parsed:
        line = diff[1].strip()
        if not line.startswith("//") and not (line.startswith("/*") and line.endswith("*/")):
            diff_filtered.append(diff)
    return diff_filtered


def rm_empty_lines(diff_parsed):
    diff_filtered = []
    for diff in diff_parsed:
        line = diff[1].strip()
        if not line == '':
            diff_filtered.append(diff)
    return diff_filtered


def clean_diff(diff_parsed):
    diff_filtered = []

    for diff in diff_parsed:
        line = diff[1]        
    
        # remove // comments (only at end of a line)
        start_comment = line.find("//")
        if start_comment != -1:
            line = line[:start_comment]

        # remove /* */ comments (can be within a line)
        start_comment = line.find("/*")
        end_comment = line.find("*/")
        while start_comment != -1 and end_comment != -1:
            line = line[:start_comment] + line[end_comment+2:]
            start_comment = line.find("/*")
            end_comment = line.find("*/")
        
        # remove content inside a string, e.g. "a \" string"
        pattern = r'(?<!\\)"'
        matches = [match.start() for match in re.finditer(pattern, line)]
        if len(matches) % 2 == 0:  # if odd something went wrong
            for i in range(len(matches), 0, -2):
                start_string = matches[i-2]
                end_string = matches[i-1]
                line = line[:start_string] + line[end_string+1:]

        # make spacing consistent
        line = line.strip()
        line = line.replace('\t', ' ')
        while '  ' in line:
            line = line.replace('  ', ' ')

        diff_filtered.append((diff[0], line))
    
    return diff_filtered


def remove_no_change_lines(diff_parsed, diff_mod):
    # make added and deleted lines match by transforming added lines
    # added line = line num + (prev deleted) - (prev added)

    # find match, if line numbers agree too, then remove from both
    remove = {'added': [], 'deleted': []}
    for num_prev_added, added in enumerate(diff_mod['added']):
        added_content = added[1]
        for num_prev_deleted, deleted in enumerate(diff_mod['deleted']):
            deleted_content = deleted[1]

            if added_content == deleted_content:
                added_line_trans = added[0] + num_prev_deleted - num_prev_added

                if added_line_trans == deleted[0]:
                    remove['added'].append(added[0])
                    remove['deleted'].append(deleted[0])

    diff_filtered = {}
    diff_filtered['added'] = [diff for diff in diff_parsed['added'] if diff[0] not in remove['added']]
    diff_filtered['deleted'] = [diff for diff in diff_parsed['deleted'] if diff[0] not in remove['deleted']]

    return diff_filtered


def diff_rm_trivial_changes(diff_parsed_og, src_before, src_after):
    diff_parsed = {}

    # remove multi-line comments from diffs
    diff_parsed['added'] = rm_multi_line_comments(diff_parsed_og['added'], src_after)
    diff_parsed['deleted'] = rm_multi_line_comments(diff_parsed_og['deleted'], src_before)

    # remove single-line comments from diffs
    diff_parsed['added'] = rm_single_line_comments(diff_parsed['added'])
    diff_parsed['deleted'] = rm_single_line_comments(diff_parsed['deleted'])

    # remove empty lines from diffs
    diff_parsed['added'] = rm_empty_lines(diff_parsed['added'])
    diff_parsed['deleted'] = rm_empty_lines(diff_parsed['deleted'])

    # remove end of line comments from diff content
    diff_mod = {}
    diff_mod['added'] = clean_diff(diff_parsed['added'])
    diff_mod['deleted'] = clean_diff(diff_parsed['deleted'])

    # remove lines same in both after cleaning
    diff_parsed = remove_no_change_lines(diff_parsed, diff_mod)

    return diff_parsed


def save_samples_each_step(samples_each_step, step_number, save_path):
    """Save the current samples_each_step to CSV files."""
    log_dir = Path(save_path)
    log_dir.mkdir(parents=True, exist_ok=True)
    
    # Save counts
    counts_filename = log_dir / f"samples_each_step_step{step_number}_counts.csv"
    with open(counts_filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Step', 'Description', 'Count'])
        for step in samples_each_step:
            for description, count in step['counts'].items():
                writer.writerow([step['step'], description, count])
    print(f"Saved counts to {counts_filename}")
    
    # Save sample IDs
    ids_filename = log_dir / f"samples_each_step_step{step_number}_ids.csv"
    with open(ids_filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Step', 'Description', 'Sample_IDs'])
        for step in samples_each_step:
            for description, ids in step['ids'].items():
                # Join IDs with a semicolon for CSV storage
                ids_str = ';'.join(ids)
                writer.writerow([step['step'], description, ids_str])
    print(f"Saved sample IDs to {ids_filename}")


def filter_commit(commit):
    files = [file for file in commit.modified_files if file.change_type == ModificationType.MODIFY and not ("test" in file.old_path.lower() or "fuzz" in file.old_path.lower())]
    if len(files) != 1:  # only one file may be modified
        return "0 or >1 modified non test/fuzz file", None
    file = files[0]
    # Uncomment the following lines if you want to enforce additions and deletions
    # if file.added_lines == 0 or file.deleted_lines == 0:  # must include additions and deletions
    #     return "0 additions or 0 deletions", None
    if len(file.changed_methods) != 1:  # only a single function modified
        return "0 or >1 functions modified", None
    return "success", (file.source_code, file.source_code_before, file.changed_methods[0].name)


def make_mangled_name(name, full_parameters):
    param_types = []
    for param in full_parameters:
        param = param.strip()
        if len(param.split(' ')) == 1:
            param_types.append(param)
        else:
            param_types.append(' '.join(param.split(' ')[:-1]))
    mangled_name = [name] + param_types
    return mangled_name


def process_sample(stem, meta, patch):
    """Process a single sample and return the result."""
    # Get commit hash
    fix_commit = meta['fix_commit']
    if isinstance(fix_commit, str):
        commit_hash = fix_commit
    else:
        # Assuming 'fix' field contains the correct commit hash
        commit_hash = next((fix_can for fix_can in fix_commit if fix_can in meta['fix']), None)
        if commit_hash is None:
            return stem, "invalid fix_commit", None

    # get url -- must be using single thread if using local repos
    local_repos = os.listdir('/home/cdilgren/project_benchmark/repos')
    if meta['project'] not in local_repos:
        url = meta["repo_addr"].rstrip("/")
    else:
        url = f'/home/cdilgren/project_benchmark/repos/{meta['project']}'

    # some times Repository fails on input that passed before. So try multiple times.
    max_retries = 3
    for i in range(max_retries):
        with tempfile.TemporaryDirectory() as temp_dir:
            try:
                if meta['project'] in local_repos:
                    commits = list(Repository(url, single=commit_hash).traverse_commits())
                else:
                    commits = list(Repository(url, single=commit_hash, clone_repo_to=temp_dir).traverse_commits())

                if not commits:
                    return stem, "no commits found", None
                commit = commits[0]
                
                # Filter out merge commits (have >1 parent)
                if len(commit.parents) != 1:
                    return stem, "merge commit", None

                # Remove samples that change !=1 cxx files
                files = []
                for file in commit.modified_files:
                    if file.change_type == ModificationType.MODIFY and is_cxx_src(file.old_path.lower()) and is_nontrivial_change(file):
                        files.append(file)
                if len(files) != 1:  # needs to have at least one file modified
                    return stem, "Changed 0 or >1 cxx files", None
                file = files[0]

                # modify diff to ignore trivial changes
                src_before = file.source_code_before.split('\n')
                src_after = file.source_code.split('\n')
                diff_parsed = diff_rm_trivial_changes(file.diff_parsed, src_before, src_after)

                # get changed method
                changed_methods = []

                added_line_nums = [diff[0] for diff in diff_parsed['added']]
                deleted_line_nums = [diff[0] for diff in diff_parsed['deleted']]
                changed_lines = [added_line_nums, deleted_line_nums]

                file_lizard_src = lizard.analyze_file.analyze_source_code(file.filename, file.source_code)
                file_lizard_src_before = lizard.analyze_file.analyze_source_code(file.filename, file.source_code_before)

                src_after_funcs = file_lizard_src.function_list
                src_before_funcs = file_lizard_src_before.function_list
                src_funcs = [src_after_funcs, src_before_funcs]

                mangled_names_after = [make_mangled_name(func.name, func.full_parameters) for func in src_after_funcs]
                mangled_names_before = [make_mangled_name(func.name, func.full_parameters) for func in src_before_funcs]

                for line_nums, funcs in zip(changed_lines, src_funcs):
                    for line_num in line_nums:
                        for func in funcs:
                            if func.start_line <= line_num <= func.end_line:
                                # make mangled name
                                mangled_name = make_mangled_name(func.name, func.full_parameters)
                                # function should be in src before and after commit
                                if mangled_name not in changed_methods and mangled_name in mangled_names_after and mangled_name in mangled_names_before:
                                    changed_methods.append(mangled_name)

                # Only a single function modified
                if len(changed_methods) != 1:
                    return stem, "0 or >1 changed functions", None

                # Successfully processed the sample
                case = {
                    "project_name": meta["project"].lower(),
                    "fixing_commit": commit.hash,
                    "changed_file": file.new_path,
                    "changed_function": changed_methods[0],
                    "diff": diff_parsed,
                    "source_code_before": file.source_code_before,
                    "source_code": file.source_code
                }
                return stem, "success", case
            except Exception as e:
                repo_error = f"Repository error: {str(e)}"
                if i+1 == max_retries:
                    return stem, repo_error, None
                time.sleep(3)
                continue


def main(save_path, parallel=True, rerun=True):

    if not Path("ARVO-Meta").is_dir():
        Repo.clone_from("https://github.com/n132/ARVO-Meta.git", "ARVO-Meta")

    # Step 1: Get starting samples
    patch_paths = list(Path("./").glob("ARVO-Meta/patches/*.diff"))
    samples = {patch_path.stem: {'patch_path': patch_path}
               for patch_path in patch_paths}
    meta_paths = list(Path("./").glob("ARVO-Meta/meta/*.json"))
    for meta_path in meta_paths:
        if meta_path.stem in samples:
            samples[meta_path.stem]['meta_path'] = meta_path

    # if rerun is False, then get cached results
    if rerun is False:
        cases_filename = Path(save_path) / 'cases.json'
        with open(cases_filename, 'r') as f:
            cases = json.load(f)

        remove = [stem for stem in cases]
        for rm in remove: del samples[rm]
    else:
        cases = {}

    samples_each_step = []
    samples_each_step.append({
        'step': 1,
        'description': 'Start',
        'number of samples present': len(samples),
        'number of samples lost': 0,
        'sample ids present': list(samples.keys()),
        'sample ids lost': []
    })

    # Step 2: Filter out samples that aren't in both meta and patches folders
    remove = [stem for stem in samples if 'meta_path' not in samples[stem]]
    for rm in remove: del samples[rm]

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 2,
        'description': 'Samples in both meta and patches folders',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # initialize proj_samples tracker
    id_2_proj = {}
    if rerun is False:
        for stem, data in cases.items():
            id_2_proj[stem] = data['project_name']
    for stem in list(samples.keys()):
        meta = json.load(samples[stem]['meta_path'].open())
        samples[stem]['meta'] = meta
        id_2_proj[stem] = meta['project'].lower()

    # Step 3: Filter out duplicates | keep first instance of commit
    if rerun is True:
        fixes = set()
    else:
        fixes = {case['fixing_commit'] for case in cases.values()}

    remove = []
    for stem in [str(id) for id in sorted([int(id) for id in list(samples.keys())])]:
        meta = samples[stem]['meta']
        # Get commit hash
        fix_commit = meta['fix_commit']
        if isinstance(fix_commit, str):
            commit_hash = fix_commit
        else:
            # Assuming 'fix' field contains the correct commit hash
            commit_hash = next((fix_can for fix_can in fix_commit if fix_can in meta['fix']), None)
        if commit_hash in fixes:
            remove.append(stem)
        fixes.add(commit_hash)
    for rm in remove:
        del samples[rm]

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 3,
        'description': 'Samples with unique fixing commits',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # Step 4: Filter out patch files with UnicodeDecodeError or UnidiffParseError
    remove = []
    for stem in list(samples.keys()):
        try:
            patch = PatchSet.from_filename(str(samples[stem]['patch_path']))
            samples[stem]['patch'] = patch
        except (UnicodeDecodeError, UnidiffParseError):
            remove.append(stem)
    for rm in remove:
        del samples[rm]

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 4,
        'description': 'Samples without UnicodeDecodeError or UnidiffParseError in diff file',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # Step 5: Filter out non-git repositories (e.g., svn)
    remove = [stem for stem in samples if 'git' not in samples[stem]['meta']["repo_addr"].lower()]
    for rm in remove:
        del samples[rm]

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 5,
        'description': 'Samples after removing non-git repositories',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # Prepare for parallel processing
    remaining_samples = samples.copy()

    # Initialize a separate list to track stats with sample IDs
    processing_stats = {
        "invalid fix_commit": [],
        "no commits found": [],
        "Repository error": [],
        "merge commit": [],
        "Changed 0 or >1 cxx files": [],
        "0 or >1 changed functions": [],
        "exception": []
    }

    if parallel is True:
        # Define the number of workers based on CPU cores
        num_workers = max(1, multiprocessing.cpu_count() // 2)  # Half the available CPUs
        print(num_workers)

        # Use ProcessPoolExecutor for parallel processing
        repository_errors = []
        with ProcessPoolExecutor(max_workers=num_workers) as executor:
            # Submit all tasks
            future_to_stem = {
                executor.submit(process_sample, stem, samples[stem]['meta'], samples[stem]['patch']): stem
                for stem in remaining_samples
            }

            # Use alive_bar to show progress
            with alive_bar(len(future_to_stem)) as bar:
                for future in as_completed(future_to_stem):
                    stem = future_to_stem[future]
                    try:
                        stem, status, case = future.result()
                        if status == "success":
                            cases[stem] = case
                        else:
                            if status == "invalid fix_commit":
                                processing_stats["invalid fix_commit"].append(stem)
                            elif status == "no commits found":
                                processing_stats["no commits found"].append(stem)
                            elif status.startswith("Repository error"):
                                processing_stats["Repository error"].append(stem)
                                repository_errors.append((stem, status))
                            elif status == "merge commit":
                                processing_stats["merge commit"].append(stem)
                            elif status == "Changed 0 or >1 cxx files":
                                processing_stats["Changed 0 or >1 cxx files"].append(stem)
                            elif status == "0 or >1 changed functions":
                                processing_stats["0 or >1 changed functions"].append(stem)
                            else:
                                processing_stats.setdefault("other issues", []).append(stem)
                    except Exception as e:
                        processing_stats["exception"].append(stem)
                    bar()
    
    else:
        # Process tasks sequentially and show progress with `alive_bar`
        repository_errors = []
        with alive_bar(len(remaining_samples)) as bar:
            for stem in remaining_samples:
                try:
                    # Process the sample
                    stem, status, case = process_sample(stem, samples[stem]['meta'], samples[stem]['patch'])
                    
                    if status == "success":
                        cases[stem] = case
                    else:
                        if status == "invalid fix_commit":
                            processing_stats["invalid fix_commit"].append(stem)
                        elif status == "no commits found":
                            processing_stats["no commits found"].append(stem)
                        elif status.startswith("Repository error"):
                            processing_stats["Repository error"].append(stem)
                            repository_errors.append((stem, status))
                        elif status == "merge commit":
                            processing_stats["merge commit"].append(stem)
                        elif status == "Changed 0 or >1 cxx files":
                            processing_stats["Changed 0 or >1 cxx files"].append(stem)
                        elif status == "0 or >1 changed functions":
                            processing_stats["0 or >1 changed functions"].append(stem)
                        else:
                            processing_stats.setdefault("other issues", []).append(stem)
                except Exception as e:
                    processing_stats["exception"].append(stem)
                bar()

    # Update samples_each_step with the results of parallel processing
    # Step 6: Samples that have a valid fix_commit
    for rm in processing_stats["invalid fix_commit"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 6,
        'description': 'Samples with valid fix_commit',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["invalid fix_commit"]
    })

    # Step 7: Samples that have a commit
    for rm in processing_stats["no commits found"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 7,
        'description': 'Samples with a commit',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["no commits found"]
    })

    # Step 8: Samples that could be read with Repository()
    for rm in processing_stats["Repository error"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 8,
        'description': 'Samples that could be read with Repository()',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["Repository error"]
    })

    # Step 9: Samples that are not merge commits
    for rm in processing_stats["merge commit"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 9,
        'description': 'Samples that are not merge commits',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["merge commit"]
    })

    # Step 10: Samples that changed 0 or >1 cxx files
    for rm in processing_stats["Changed 0 or >1 cxx files"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 10,
        'description': 'Samples that changed 0 or >1 cxx files',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["Changed 0 or >1 cxx files"]
    })

    # Step 11: Samples with 0 or >1 changed functions
    for rm in processing_stats["0 or >1 changed functions"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 11,
        'description': 'Samples with 1 changed function',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["0 or >1 changed functions"]
    })

    # Step 12: Samples with some other exception
    for rm in processing_stats["exception"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 12,
        'description': 'Samples without an exeception',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["exception"]
    })

    # Step 13: Samples that are in projects.py (we can compile and test)
    projects_unittest = set(unittest_commands.keys())
    projects_patterns = set(unittest_patterns.keys())
    projects_bad = set(bad_projects)
    projects_valid = projects_unittest.intersection(projects_patterns).difference(projects_bad)
    projects_valid = [proj.lower() for proj in projects_valid]
    remove = []
    for stem in samples:
        project = samples[stem]['meta']['project'].lower()
        if project not in projects_valid:
            remove.append(stem)
    for rm in remove: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 13,
        'description': 'Samples in projects.py',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # Step 14: Samples that are in top 40
    project_freq = defaultdict(int)
    for stem in samples:
        project = samples[stem]['meta']['project'].lower()
        project_freq[project] += 1
    sorted_proj = sorted(project_freq.items(), key=lambda item: item[1], reverse=True)
    top_40 = [proj[0] for proj in sorted_proj[:40]]
    remove = []
    for stem in samples:
        if samples[stem]['meta']['project'].lower() not in top_40:
            remove.append(stem)
    for rm in remove: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 14,
        'description': 'Samples in top 40',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # make save_path
    if not Path(save_path).exists():
        Path(save_path).mkdir(exist_ok=True)

    # Save final cases
    cases_filename = Path(save_path) / 'cases.json'
    with open(cases_filename, 'w') as f:
        json.dump(cases, f, indent=4)
    print(f"Saved cases to {cases_filename}")

    # save samples at each step
    samples_each_step_filename = Path(save_path) / 'samples_each_step.json'
    if rerun is False:
        with open(samples_each_step_filename, 'r') as f:
            samples_each_step_prev = json.load(f)
        for prev, cur in zip(samples_each_step_prev, samples_each_step):
            cur['sample ids present'] = list(set(cur['sample ids present']).union(set(prev['sample ids present'])))
            cur['number of samples present'] = len(cur['sample ids present'])
    
    with open(samples_each_step_filename, 'w') as f:
        json.dump(samples_each_step, f, indent=4)
    print(f"Saved samples at each step to {samples_each_step_filename}")

    # Save final counts and IDs
    final_counts_filename = Path(save_path) / "samples_each_step_final_counts.csv"
    with open(final_counts_filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['Step', 'Description', 'Number of Samples Present'])
        for step in samples_each_step:
            writer.writerow([step['step'], step['description'], step['number of samples present']])
    print(f"Saved final counts to {final_counts_filename}")

    # save proj_samples
    proj_samples_filename = Path(save_path) / "proj_samples.csv"
    proj_samples = create_proj_samples(samples_each_step, id_2_proj)
    proj_samples.to_csv(proj_samples_filename)
    print(f"Saved the number of samples per project at each filtering step to {proj_samples_filename}")

    # save repo read errors
    repo_read_errors = Path(save_path) / "repo_read_errors.csv"
    with open(repo_read_errors, 'w') as f:
        json.dump(repository_errors, f, indent=4)
    print(f"Saved repository read errors to {repo_read_errors}")

    # save top 40 proejcts
    top_40_projects = Path(save_path) / "top_40.json"
    with open(top_40_projects, 'w') as f:
        json.dump(top_40, f, indent=4)
    print(f"Saved top 40 projects to {top_40_projects}")


if __name__ == "__main__":
    save_path = "filter_logs"
    main(save_path, parallel=False, rerun=True)
