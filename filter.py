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

def is_cxx_src(path):
    path = path.lower()
    for f in ["fuzz", "test"]:
        if f in path:
            return False
    if True in [ext == path.split(".")[-1] for ext in ["c", "cc", "cpp", "cxx", "h", "hh", "hpp"]]:
        return True
    return False

def filter_case(path):
    try:
        local_id = path.stem
        patch = PatchSet.from_filename(path)
        meta = json.load(Path(f"./ARVO-Meta/meta/{local_id}.json").open())
        if isinstance(meta["fix_commit"], list): # only a single fixing commit is allowed
            return ">1 fixing commits"
        files = [f for f in patch if is_cxx_src(f.path)] # only the non testing files are included
        if len(files) != 1: # only one file must be modified
            return "0 or >1 modified C/C++ file"
        file = files[0]
        #if file.added < 1 or file.removed < 1: # file must include additions and deletions
        #    return "0 additions or 0 deletions"
        #analysis = lizard.analyze_file.analyze_source_code(file.path, str(file))
        #if len(analysis.function_list) != 1:
        #    return False
        #if len(list(file)) > 1: # there must only be one hunk
        #    return False
        return local_id, file, patch, meta
    except (UnidiffParseError, UnicodeDecodeError, TypeError, FileNotFoundError) as e:
        return "exception " + str(e)

def filter_commit(commit):
    files = [file for file in commit.modified_files if file.change_type == ModificationType.MODIFY and not ("test" in file.old_path.lower() or "fuzz" in file.old_path.lower())]
    if len(files) != 1: # only one file may be modified
        return "0 or >1 modified non test/fuzz file"
    file = files[0]
    #if file.added_lines == 0 or file.deleted_lines == 0: # must include additions and deletions
    #    return "0 additions or 0 deletions"
    if len(file.changed_methods) != 1: # only a single function modified
        return "0 or >1 functions modified"
    return file.source_code, file.source_code_before, file.changed_methods[0].name
    
    
def get_diff_dict(file):
    diff_dict = {"added": [], "deleted": []}
    for hunk in file:
        for line in hunk:
            if line.is_added:
                diff_dict["added"].append([line.target_line_no, line.value.strip()])
            elif line.is_removed:
                diff_dict["deleted"].append([line.source_line_no, line.value.strip()])
    return diff_dict     

def main():

    if not Path("ARVO-Meta").is_dir():
        Repo.clone_from("https://github.com/n132/ARVO-Meta.git", "ARVO-Meta")

    commits = {}
    cases = {}
    stats = {}
    paths = list(Path("./").glob("ARVO-Meta/patches/*.diff"))
    with alive_bar(len(paths)) as bar:
        for diff_path in paths:
            try:
                local_id = diff_path.stem
                res = filter_case(diff_path)
                if isinstance(res, tuple):
                    local_id, file, patch, meta = res
                    url = meta["repo_addr"].rstrip("/")
                    for traversed_case in cases.values():
                        print(meta["project"].lower())
                        print(traversed_case["project_name"])
                        print(meta["fix_commit"])
                        print(traversed_case["fixing_commit"])
                        if meta["project"].lower() == traversed_case["project_name"] and meta["fix_commit"] == traversed_case["fixing_commit"]:
                            print("!!!")
                            res = "duplicate commit"
                            break
                    if not isinstance(res, str):  
                        for commit in Repository(url, single=meta["fix_commit"]).traverse_commits():
                            res = filter_commit(commit)
                            if isinstance(res, tuple):
                                sec, vul, method = res
                                #print(commit.hash)
                                #print(commit.hash in commits)
                                cases[local_id] = {
                                    "project_name":meta["project"].lower(),
                                    "fixing_commit":commit.hash, 
                                    "changed_file":file.path, 
                                    "changed_function": method,
                                    "diff":get_diff_dict(file),
                                    "source_code_before":vul,
                                    "source_code":sec
                                }
                                res = "success"
            except Exception as e:
                res = "exception " + str(e)
            if res in stats:
                stats[res].append(diff_path.stem)
            else:
                stats[res] = [diff_path.stem] 
            #print(stats)
            bar()
    
    json.dump(stats, open("stats.json", "w"), indent=4)
    json.dump(cases, open("cases.json", "w"), indent=4)


if __name__ == "__main__":
    main() 