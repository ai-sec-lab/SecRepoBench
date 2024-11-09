import sys
import subprocess
import argparse
import json
import re
from pathlib import Path
from base64 import b64decode
from multiprocessing import Pool, cpu_count
from functools import partial
from alive_progress import alive_bar
import pickle
from datetime import datetime
import time
import random

from projects import * 

def load_txt(path):
    """
    load txt data and return as a dict
    """
    data = {}
    with open(path, "r") as file:
        for line in file.read().strip().split("\n"):
            (
                local_id, 
                project_name, 
                fixed_range,
                fixing_commit, 
                commit_message, 
                changed_file, 
                changed_function,
                diff_json, 
                source_code_before, 
                source_code
            ) = line.split("\t")
            skip = False
            if local_id in data.keys():
                print(f"duplicate local_id {local_id} found in {path}")
                skip = True
            for new_local_id, entry in data.items():
                if fixing_commit == entry["fixing_commit"]:
                    print(f"duplicate fixing commit for {local_id}, {new_local_id} found in {path}")
                    skip = True
            if not skip:
                try:
                    data[local_id] = {
                        "project_name":project_name,
                        "fixed_range":fixed_range,
                        "fixing_commit":fixing_commit, 
                        "commit_message":eval(commit_message), 
                        "changed_file":changed_file, 
                        "changed_function":changed_function,
                        "diff":json.loads(diff_json), 
                        "source_code_before":b64decode(eval(source_code_before)).decode(),
                        "source_code":b64decode(eval(source_code)).decode()
                    }
                except Exception as e:
                    print(f"error with {local_id} {e}")

    print(f"{len(data)} cases loaded from {path}")
    return data


def setup(local_id, project_name, patch_path, diff, vul_content, sec_content, root="."):

    directory = Path(root) / str(local_id)
    dockerfile_dir = directory / "Dockerfile"
    testcase_dir = directory / "testcase.sh"
    unittest_dir = directory / "unittest.sh"
    vul_dir = directory / "patches" / "vul.txt"
    sec_dir = directory / "patches" / "sec.txt"
    err_dir = directory / "patches" / "err.txt"

    local_id = "35422" if local_id == "35407" else local_id
    #local_id = "39343" if local_id == "39329" else local_id

    proc = subprocess.run(["docker", "pull", f"n132/arvo:{local_id}-fix"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if proc.returncode != 0:
        return False

    scripts_content = (
        f"#!/bin/bash\n"
        f"docker build {directory.absolute()} -t oss-fuzz-bench:{local_id}-$1\n"
        f"docker run --rm oss-fuzz-bench:{local_id}-$1 /bin/sh -c \""
        f"mv -f /patches/$1.txt \\$(find /src -type d -iname '{project_name}' | head -n 1)/{patch_path} && "
    )
    dockerfile_content = (
        f"FROM n132/arvo:{local_id}-fix\n"
        f"ADD patches /patches\n"
    )
    testcase_content = scripts_content + "arvo compile ; arvo run\""
    unittest_content = scripts_content + (unittest_commands[project_name.lower()] if project_name in unittest_commands else "echo 'NO UNIT TESTS'") + "\""
    #err_content = gen_err_patch(diff, vul_content, sec_content)

    directory.mkdir(exist_ok=True, parents=True)
    (directory / "patches").mkdir(exist_ok=True)
    dockerfile_dir.open("w").write(dockerfile_content)
    testcase_dir.open("w").write(testcase_content)
    unittest_dir.open("w").write(unittest_content)
    vul_dir.open("w").write(vul_content)
    sec_dir.open("w").write(sec_content)
    #err_dir.open("w").write(err_content)
    
    subprocess.run(["chmod", "-R",  "777", str(directory.absolute())])
    return True
    
def get_targets(local_id, filter_patches, root="./"):

    directory = Path(root) / str(local_id)
    patches = [p.stem for p in (directory / "patches").glob("./*.txt") if filter_patches in p.stem]
    testcase_targets = [["/bin/bash", (directory / "testcase.sh").absolute(), patch] for patch in patches]
    unittest_targets = [["/bin/bash", (directory / "unittest.sh").absolute(), patch] for patch in patches]
    targets = []
    for patch in patches:
        targets.append((local_id, patch, "testcase", ["/bin/bash", (directory / "testcase.sh").absolute(), patch]))
        targets.append((local_id, patch, "unittest", ["/bin/bash", (directory / "unittest.sh").absolute(), patch]))
    return targets

class ParseException(Exception):
    pass

def parse_testcase(output):
    local_id, patch, test_type, proc = output
    stderr = proc.stderr.decode(errors='ignore')
    stdout = proc.stdout.decode(errors='ignore')

    # cases that always mean pass
    if re.search(r"NOTE: fuzzing was not performed", stderr):
        if proc.returncode == 0:
            return "pass"
        raise ParseException(f"crash detected with zero return code ({proc.returncode})")

    # cases that sometimes mean crash
    elif re.search(r"(==\d+==\s?[A-Z]*|runtime error):([\s\S]*?)(Exiting|ABORTING|$)", stderr):
        if proc.returncode != 0:
            return "crash"
        else:
            raise ParseException(f"crash detected with zero return code ({proc.returncode})")

        # cases that sometimes mean error
    elif (
            re.search(r"make(\[\d+\])?:\s\*\*\*\s\[.*\]\sError\s\d+", stderr) or
            re.search(r"clang-\d+:\serror:.*", stderr) or 
            re.search(r"ninja: build stopped: subcommand failed.", stdout)
        ):
        raise ParseException(f"compile error ({proc.returncode})")
    
    # cases that sometimes mean pass
    elif (
            re.search(r"NOTE: fuzzing was not performed", stderr) or
            re.search(r"Usage for fuzzing: honggfuzz", stderr) or
            re.search(r"This binary is built for AFL-fuzz\.", stderr) or
            re.search(r"Execution successful\.", stdout) or 
            re.search(r"make.*: Leaving directory .*$", stdout)
        ):
        if proc.returncode == 0:
            return "pass"
        raise ParseException(f"no crash detected with non zero return code ({proc.returncode})")

    else:
        raise ParseException(f"no matching regex case ({proc.returncode})")

def parse_unittest(output, project_name):
    project_name = project_name.lower()

    local_id, patch, test_type, proc = output
    stderr = proc.stderr.decode(errors='ignore')
    stdout = proc.stdout.decode(errors='ignore')

    result = {
        "pass":  [], # list of str
        "fail":  [], # list of strs or int
        "skip":  [], # list of strs or int
        "total": None  # int
    }
    
    if not project_name in unittest_patterns:
        raise ParseException(f"no pattern for {project_name}")

    patterns = unittest_patterns[project_name]
    if not isinstance(patterns, list):
        patterns = [patterns]

    for pattern in patterns:
        for test in re.finditer(pattern, stdout + "\n" + stderr):
            if local_id == "66696" and test.group("name") == "sock-tcp-raw-raw": # this case causes errors between the two versions
                continue
            for g in ["name", "total"]:
                if g in list(test.re.groupindex.keys()) and test.group(g) != None:
                    if g == "total":
                        if result["total"] == None:
                            result["total"] = 0
                        if test.group("total").isdigit():
                            result["total"] += int(test.group("total"))
                        else:
                            result["total"] += 1
                    elif g == "name":
                        if "status" in list(test.re.groupindex.keys()) and test.group("status") != None: # if there is a status, use that
                            s = test.group("status").lower().strip()
                            s = "pass" if s in ["ok", "okay", "success", ".", "", "done"] else s
                            s = "fail" if s in ["error", "e", "f", "fail"] else s
                            s = "skip" if s in ["?"] else s
                        else: # otherwise, the default status is pass
                            s = "pass"
                        for status in ["pass", "fail", "skip"]:
                            if status in s:
                                if result[status] == None:
                                    result[status] = []
                                result[status].append(test.group("name"))

    if result["total"] == None:
        result["total"] = sum([len(result[s]) if isinstance(result[s], list) else result[s] for s in ["pass", "fail"]])
    return result


def parse_output(output, project_name, report={}, root="./"):
    local_id, patch, test_type, proc_data = output

    if test_type == "testcase":
        try:
            result = parse_testcase((local_id, patch, test_type, type('Proc', (), proc_data)()))
        except ParseException as e:
            result = "error: " + str(e)
        
    elif test_type == "unittest":
        try:
            result = parse_unittest((local_id, patch, test_type, type('Proc', (), proc_data)()), project_name)
        except ParseException as e:
            result = "error: " + str(e)
        
    if str(local_id) in report:
        if patch in report[str(local_id)]:
            report[str(local_id)][patch][test_type] = result
        else:
             report[str(local_id)][patch] = {test_type:result}
    else:
        report[str(local_id)] = {patch:{test_type:result}}

    return report

def print_report(report):
    for local_id, patches in report.items():
        print(local_id)
        for patch_type, results in patches.items():
            if "unittest" in results:
                if isinstance(results['unittest'], dict):
                    if isinstance(results['unittest']['fail'], list):
                        num_fail = len(results['unittest']['fail'])
                    else:
                        num_fail = results['unittest']['fail']
                
                    pass_over_total = f"{results['unittest']['total'] - num_fail}/{results['unittest']['total']}"
                else:
                    pass_over_total = results['unittest']
            else:
                pass_over_total = "N/A"
            if 'testcase' in results:
                tcr = results['testcase']
            else:
                tcr = "N/A"
            print(f"{patch_type}\t{tcr}\t{pass_over_total}")


def write_output(output, root="./"):
    local_id, patch, test_type, proc = output
    directory = Path(root) / str(local_id) / str(patch) / test_type
    directory.mkdir(exist_ok=True, parents=True)
    (directory / "stdout.txt").open("wb").write(proc["stdout"])
    (directory / "stderr.txt").open("wb").write(proc["stderr"])
    
    # Cache the raw output
    cache_file = directory / "cache.pkl"
    with cache_file.open("wb") as f:
        pickle.dump({
            "stdout": proc["stdout"],
            "stderr": proc["stderr"],
            "returncode": proc["returncode"],
            "timestamp": datetime.now().isoformat()
        }, f)

def proc_runner(target_with_output_path_and_rerun):
    target, output_path, rerun = target_with_output_path_and_rerun
    local_id, patch, test_type, cmd = target
    print(f"Running {local_id} {patch} {test_type}")
    
    # Check if cached result exists
    cache_file = Path(output_path) / str(local_id) / str(patch) / test_type / "cache.pkl"
    if cache_file.exists() and not rerun:
        try:
            with cache_file.open("rb") as f:
                cached_data = pickle.load(f)
            
            # Check if the cached result was a rate limit error
            if "429 Too Many Requests" not in cached_data.get('stderr', b'').decode():
                print(f"Using cached result for {local_id} {patch} {test_type}")
                return local_id, patch, test_type, cached_data
            else:
                print(f"Retrying rate-limited case: {local_id} {patch} {test_type}")
        except:
            pass
    
    # If no cache, cached result was a rate limit error, or rerun is True, run the process with retry
    max_retries = 5
    base_delay = 60  # 1 minute
    for attempt in range(max_retries):
        try:
            proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, timeout=1500)
        except subprocess.TimeoutExpired:
            return local_id, patch, test_type, {
                "stdout": b"",
                "stderr": b"Timeout",
                "returncode": -1
            }
        if "429 Too Many Requests" not in proc.stderr.decode(errors='ignore'):
            break
        if attempt < max_retries - 1:
            delay = base_delay * (2 ** attempt) + random.uniform(0, 10)
            print(f"Rate limit reached. Retrying in {delay:.2f} seconds...")
            time.sleep(delay)
    else:
        print(f"Failed to run {local_id} {patch} {test_type} after {max_retries} attempts")

    print(f"Finished {local_id} {patch} {test_type}")
    return local_id, patch, test_type, {
        "stdout": proc.stdout,
        "stderr": proc.stderr,
        "returncode": proc.returncode
    }

def get_remaining(targets, completed):
    return [target for target in targets if (target[0], target[1], target[2]) not in completed]

def main():
    cases_fname = "cases.json"

    # Define the set of predefined actions
    actions = ['load', 'setup', 'eval']
    parser = argparse.ArgumentParser(description="A program that performs actions on targets.")
    parser.add_argument('action', choices=actions, help="The action to perform. Must be one of: " + ", ".join(actions))
    parser.add_argument('targets', nargs='+', help='Targets as one or more integers or the string "all".')
    parser.add_argument("-p", "--path", type=str, help="", default="./")
    parser.add_argument("-o", "--output", type=str, help="", default="./")
    parser.add_argument("-f", "--filter-patches", type=str, help="only evaluate patches including a substring", default="")
    parser.add_argument("--rerun", action="store_true", help="Rerun targets even if they have cached results")

    args = parser.parse_args()

    root = Path(args.path)

    if args.action == "load":
        data = {}
        for path in args.targets:
            for local_id, entry in load_txt(Path(path)).items():
                data[local_id] = entry
        json.dump(data, open(cases_fname, "w"))
    else:
        data = json.load(open(cases_fname, "r"))

        if 'all' in args.targets:
            if len(args.targets) > 1:
                parser.error('If "all" is used, it must be the only target.')
            targets = list(data.keys())
        else:
            try:
                targets = [int(t) for t in args.targets]
            except ValueError:
                parser.error('Targets must be integers or "all".')

        if args.action == "setup":
            with alive_bar(len(targets)) as bar:
                for local_id in targets:
                    local_id = str(local_id)
                    if not setup(local_id, data[local_id]["project_name"], data[local_id]["changed_file"], data[local_id]["diff"], data[local_id]["source_code_before"], data[local_id]["source_code"], root=root):
                        print(f"{local_id} failed")
                    bar()

        if args.action == "eval":
            int_targets = targets
            targets = [c for target in targets for c in get_targets(target, args.filter_patches, root=root)]
            
            # Load completed targets from a file if it exists
            completed_file = Path(args.output) / "completed_targets.pkl"
            if completed_file.exists() and not args.rerun:
                with completed_file.open("rb") as f:
                    completed = pickle.load(f)
            else:
                completed = set()

            # Count cached results and identify rate-limited cases
            cached_count = 0
            rate_limited_count = 0
            for target in targets:
                local_id, patch, test_type, _ = target
                cache_file = Path(args.output) / str(local_id) / str(patch) / test_type / "cache.pkl"
                if cache_file.exists() and not args.rerun:
                    try:
                        with cache_file.open("rb") as f:
                            cached_data = pickle.load(f)
                        if "429 Too Many Requests" in cached_data.get('stderr', b'').decode(errors="ignore"):
                            rate_limited_count += 1
                            # Remove from completed set to ensure it's rerun
                            completed.discard((local_id, patch, test_type))
                        else:
                            cached_count += 1
                    except:
                        pass

            print(f"Found {cached_count} valid cached results and {rate_limited_count} rate-limited cases out of {len(targets)} total targets.")
            if args.rerun:
                print("Rerunning all targets, including those with cached results.")

            procs = []
            pool_size = min(int(cpu_count() / 3), len(targets))
            try:
                with alive_bar(len(targets)) as bar, Pool(pool_size) as p:
                    remaining_targets = get_remaining(targets, completed) if not args.rerun else targets
                    targets_with_output = [(target, args.output, args.rerun) for target in remaining_targets]
                    for proc in p.imap_unordered(proc_runner, targets_with_output):
                        procs.append(proc)
                        completed.add((proc[0], proc[1], proc[2]))
                        bar()

                        if len(completed) % 10 == 0:
                            with completed_file.open("wb") as f:
                                pickle.dump(completed, f)

            except KeyboardInterrupt:
                print("Interrupted. Processing all results...")
            finally:
                with completed_file.open("wb") as f:
                    pickle.dump(completed, f)

            # Process all targets, including cached ones
            report = {}
            with alive_bar(len(targets)) as bar:
                for target in targets:
                    local_id, patch, test_type, _ = target
                    cache_file = Path(args.output) / str(local_id) / str(patch) / test_type / "cache.pkl"
                    
                    if cache_file.exists() and not args.rerun:
                        try:
                            with cache_file.open("rb") as f:
                                cached_data = pickle.load(f)
                            output = (local_id, patch, test_type, cached_data)
                        except:
                            output = next((p for p in procs if p[:3] == (local_id, patch, test_type)), None)
                    else:
                        # Find the corresponding output in procs
                        output = next((p for p in procs if p[:3] == (local_id, patch, test_type)), None)
                    
                    if output:
                        parse_output(output, data[str(local_id)]["project_name"], report=report)
                        write_output(output, root=args.output)

                    bar()

            json.dump(report, (Path(args.output) / "report.json").open("w"), indent=4)
            print_report(report)

if __name__ == "__main__":
    main()


