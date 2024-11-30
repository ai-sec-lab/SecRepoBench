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
    if any(ext == path.split(".")[-1] for ext in ["c", "cc", "cpp", "cxx", "h", "hh", "hpp"]):
        return True
    return False


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
    url = meta["repo_addr"].rstrip("/")
    if meta['project'].lower() == 'binutils-gdb':
        url = './local_repos/binutils-gdb'
    elif meta['project'].lower() == 'elfutils':
        url = './local_repos/elfutils'

    # some times Repository fails on input that passed before. So try multiple times.
    max_retries = 3
    for i in range(max_retries):
        with tempfile.TemporaryDirectory() as temp_dir:
            try:
                commits = list(Repository(url, single=commit_hash, clone_repo_to=temp_dir).traverse_commits())

                if not commits:
                    return stem, "no commits found", None
                commit = commits[0]
                
                # Filter out merge commits (have >1 parent)
                if len(commit.parents) != 1:
                    return stem, "merge commit", None

                # The single cxx file must have ModificationType.MODIFY
                files = []
                for file in commit.modified_files:
                    if file.change_type == ModificationType.MODIFY and is_cxx_src(file.old_path.lower()):
                        files.append(file)
                if len(files) != 1:  # only one file may be modified
                    return stem, "Changed cxx file is not a ModificationType.MODIFY", None
                file = files[0]

                # Only a single function modified
                if len(file.changed_methods) != 1:
                    return stem, "0 or >1 changed functions", None

                # Successfully processed the sample
                case = {
                    "project_name": meta["project"].lower(),
                    "fixing_commit": commit.hash,
                    "changed_file": file.new_path,
                    "changed_function": file.changed_methods[0].name,
                    "diff": file.diff_parsed,
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
    for stem in list(samples.keys()):
        meta = samples[stem]['meta']
        fix = meta['fix']
        if fix in fixes:
            remove.append(stem)
        else:
            fixes.add(fix)
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

    # Step 5: Filter out patches that change 0 or >1 code files
    remove = []
    for stem in list(samples.keys()):
        patch = samples[stem]['patch']
        changed_files = []
        for f in patch:
            if f.source_file == None or f.target_file == None:
                continue
            if is_cxx_src(f.path):
                changed_files.append(f)
        if len(changed_files) != 1:
            remove.append(stem)
    for rm in remove:
        del samples[rm]

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 5,
        'description': 'Samples with one changed C/C++ file',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # Step 6: Filter out non-git repositories (e.g., svn)
    remove = [stem for stem in samples if 'git' not in samples[stem]['meta']["repo_addr"].lower()]
    for rm in remove:
        del samples[rm]

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 6,
        'description': 'Samples after removing non-git repositories',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': remove
    })

    # Step 7: Prepare for parallel processing
    remaining_samples = samples.copy()

    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 7,
        'description': 'Samples ready for processing',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': []
    })

    # Initialize a separate list to track stats with sample IDs
    processing_stats = {
        "invalid fix_commit": [],
        "no commits found": [],
        "Repository error": [],
        "merge commit": [],
        "Changed cxx file is not a ModificationType.MODIFY": [],
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
                            elif status == "Changed cxx file is not a ModificationType.MODIFY":
                                processing_stats["Changed cxx file is not a ModificationType.MODIFY"].append(stem)
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
                        elif status == "Changed cxx file is not a ModificationType.MODIFY":
                            processing_stats["Changed cxx file is not a ModificationType.MODIFY"].append(stem)
                        elif status == "0 or >1 changed functions":
                            processing_stats["0 or >1 changed functions"].append(stem)
                        else:
                            processing_stats.setdefault("other issues", []).append(stem)
                except Exception as e:
                    processing_stats["exception"].append(stem)
                bar()

    # Update samples_each_step with the results of parallel processing
    # Step 8: Samples that have a valid fix_commit
    for rm in processing_stats["invalid fix_commit"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 8,
        'description': 'Samples with valid fix_commit',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["invalid fix_commit"]
    })

    # Step 9: Samples that have a commit
    for rm in processing_stats["no commits found"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 9,
        'description': 'Samples with a commit',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["no commits found"]
    })

    # Step 10: Samples that could be read with Repository()
    for rm in processing_stats["Repository error"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 10,
        'description': 'Samples that could be read with Repository()',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["Repository error"]
    })

    # Step 11: Samples that are not merge commits
    for rm in processing_stats["merge commit"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 11,
        'description': 'Samples that are not merge commits',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["merge commit"]
    })

    # Step 12: Samples whose C/C++ file is a ModificationType.MODIFY
    for rm in processing_stats["Changed cxx file is not a ModificationType.MODIFY"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 12,
        'description': 'Samples whose C/C++ file is a ModificationType.MODIFY',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["Changed cxx file is not a ModificationType.MODIFY"]
    })

    # Step 13: Samples with 0 or >1 changed functions
    for rm in processing_stats["0 or >1 changed functions"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 13,
        'description': 'Samples with 1 changed function',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["0 or >1 changed functions"]
    })

    # Step 14: Samples with some other exception
    for rm in processing_stats["exception"]: del samples[rm]
    num_samples_lost = samples_each_step[-1]['number of samples present'] - len(samples)
    samples_each_step.append({
        'step': 14,
        'description': 'Samples without an exeception',
        'number of samples present': len(samples),
        'number of samples lost': num_samples_lost,
        'sample ids present': list(samples.keys()),
        'sample ids lost': processing_stats["exception"]
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


if __name__ == "__main__":
    save_path = "/space1/cdilgren/project_benchmark/filter_logs"
    main(save_path, parallel=True, rerun=False)
