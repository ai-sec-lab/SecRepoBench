import subprocess
import json
import re
from pathlib import Path
from multiprocessing import Pool
from alive_progress import alive_bar
import pickle
from datetime import datetime
import time
import random
from concurrent.futures import ProcessPoolExecutor, as_completed
import os
from collections import defaultdict
import selectors

from projects import *


def get_c_cpp_file(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        path = c_path
    elif os.path.exists(cpp_path):
        path = cpp_path
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return
    with open(path, 'r') as f:
        content = f.read()
    return content


def make_patched_file(id, model_name, context_type, prompt_type, mode):
    # get sec file base
    sec_mask_content = get_c_cpp_file(f'descriptions/{id}/mask_{mode}')

    # get code completion
    code_completion_file = f'completions/{id}/{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_code_completion.txt'
    with open(code_completion_file, 'r') as f:
        code_completion = f.read()

    # create mod file (sec file base with the LM patch)
    mod_file_content = sec_mask_content.replace("// <MASK>", code_completion)
    
    return mod_file_content


def get_docker_image(id):
    # only pull docker image if we don't already have it
    image_name = f"n132/arvo:{id}-fix"

    proc_check = subprocess.run(
        ["docker", "image", "inspect", image_name],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    if proc_check.returncode != 0:
        # Image does not exist locally, pull it
        proc_pull = subprocess.run(
            ["docker", "pull", image_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        if proc_pull.returncode != 0:
            return False
    
    return True


def setup(id, project_name, changed_file, fixing_commit, model_name, context_type, prompt_type, mode):

    mod_file_content = make_patched_file(id, model_name, context_type, prompt_type, mode)

    base_dir = os.getcwd()

    directory = Path(base_dir) / 'data' / str(id)
    directory.mkdir(parents=True, exist_ok=True)

    patch_dir = directory / "patches"
    patch_dir.mkdir(parents=True, exist_ok=True)

    testcase_file = directory / f"testcase_{model_name}_filled_code_{context_type}_{prompt_type}_{mode}.sh"
    unittest_file = directory / f"unittest_{model_name}_filled_code_{context_type}_{prompt_type}_{mode}.sh"
    patch_file_name = f"patch_{model_name}_filled_code_{context_type}_{prompt_type}_{mode}.txt"

    # write testcase, unittest bash scripts
    testcase_content = (
        f"#!/bin/bash\n"
        "docker run --rm --init "
        f"--name {id}_{model_name}_{context_type}_{prompt_type}_{mode}_testcase "
        "--cpus=2 "
        "-e MAKEFLAGS=\"-j3\" "
        f"-v {base_dir}/data/{id}/patches:/patches "
        f"n132/arvo:{id}-fix /bin/sh -c \"\n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:\\$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"anonymous@email.com\\\"\n"
        "  if [ -n \\\"\\$(git -C \\$GIT_DIR status --porcelain)\\\" ]; then\n"
        "    git -C \\$GIT_DIR stash save --include-untracked \\\"Saving my changes\\\"\n"
        "    CHANGES_STASHED=true\n"
        "  else\n"
        "    CHANGES_STASHED=false\n"
        "  fi\n"
        f"  git -C \\$GIT_DIR checkout {fixing_commit}\n"
        "  if [ \\\"\\$CHANGES_STASHED\\\" = true ]; then\n"
        "    git -C \\$GIT_DIR stash apply\n"
        "  fi\n"
        # move patched file
        f"  cp -f /patches/{patch_file_name} \\$GIT_DIR/{changed_file}\n"
        # retry loop for arvo compile
        "  ATTEMPTS=0\n"
        "  MAX_ATTEMPTS=3\n"
        "  SUCCESS=false\n"
        "  while [ \\$ATTEMPTS -lt \\$MAX_ATTEMPTS ]; do\n"
        "    ATTEMPTS=\\$((ATTEMPTS+1))\n"
        "    echo \\\"Attempt #\\$ATTEMPTS: Running arvo compile...\\\"\n"
        "    arvo compile\n"
        "    EXIT_CODE=\\$?\n"
        "    if [ \\$EXIT_CODE -eq 0 ]; then\n"
        "      echo \\\"arvo compile succeeded on attempt #\\$ATTEMPTS\\\"\n"
        "      SUCCESS=true\n"
        "      break\n"
        "    else\n"
        "      echo \\\"arvo compile failed (exit code: \\$EXIT_CODE), retrying...\\\"\n"
        "      sleep 2\n"
        "    fi\n"
        "  done\n"
        "  if [ \\\"\\$SUCCESS\\\" = false ]; then\n"
        "    echo \\\"arvo compile failed after \\$MAX_ATTEMPTS attempts. Exiting.\\\"\n"
        "    exit 1\n"
        "  fi\n"
        # run the security testcase
        "  arvo run\n"
        "  \""
    )

    unittest_content = (
        f"#!/bin/bash\n"
        "docker run --rm --init "
        f"--name {id}_{model_name}_{context_type}_{prompt_type}_{mode}_unittest "
        "--cpus=2 "
        "-e MAKEFLAGS=\"-j3\" "
        f"-v {base_dir}/data/{id}/patches:/patches "
        f"n132/arvo:{id}-fix /bin/sh -c \"\n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:\\$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"anonymous@email.com\\\"\n"
        "  if [ -n \\\"\\$(git -C \\$GIT_DIR status --porcelain)\\\" ]; then\n"
        "    git -C \\$GIT_DIR stash save --include-untracked \\\"Saving my changes\\\"\n"
        "    CHANGES_STASHED=true\n"
        "  else\n"
        "    CHANGES_STASHED=false\n"
        "  fi\n"
        f"  git -C \\$GIT_DIR checkout {fixing_commit}\n"
        "  if [ \\\"\\$CHANGES_STASHED\\\" = true ]; then\n"
        "    git -C \\$GIT_DIR stash apply\n"
        "  fi\n"
        # move patched file
        f"  cp -f /patches/{patch_file_name} \\$GIT_DIR/{changed_file}\n"
        # run unittests
        "  " + (unittest_commands[project_name.lower()] if project_name in unittest_commands else "  echo 'NO UNIT TESTS'") + "\n"
        "  \""
    )

    with open(testcase_file, 'w') as f:
        f.write(testcase_content)

    with open(unittest_file, 'w') as f:
        f.write(unittest_content)

    patch_file = patch_dir / patch_file_name
    with open(patch_file, 'w') as f:
        f.write(mod_file_content)

    return True


def eval_setup(ids, model_names, prompt_types, context_types, modes, num_workers):
    # get any necessary docker images
    print("Downloading any missing docker images")
    with ProcessPoolExecutor(max_workers=num_workers) as executor:
        future_to_stem = {
            executor.submit(
                get_docker_image,
                id
            ): id
            for id in ids
        }
        with alive_bar(len(future_to_stem)) as bar:
            for future in as_completed(future_to_stem):
                id = future_to_stem[future]
                result = future.result()
                if result is not True:
                    print(f'Could not download docker image n132/arvo:{id}-fix')
                bar()
    
    # create LLM-patched files, and bash scripts to run security and correctness tests
    print("Setting up patched files, and bash scripts to run security and correctness tests")
    combs = []
    for id in ids:
        for model_name in model_names:
            for prompt_type in prompt_types:
                for context_type in context_types:
                    for mode in modes:
                        combs.append({
                            'id': id, 
                            'model_name': model_name, 
                            'context_type': context_type, 
                            'prompt_type': prompt_type,
                            'mode': mode
                        })

    # load in sample_metadata.json
    sample_metadata = json.load(open('sample_metadata.json', "r"))

    num_workers = min(num_workers, len(combs))

    with ProcessPoolExecutor(max_workers=num_workers) as executor:
        future_to_stem = {
            executor.submit(
                setup,
                comb['id'],
                sample_metadata[comb['id']]["project_name"],
                sample_metadata[comb['id']]["changed_file"],
                sample_metadata[comb['id']]["fixing_commit"],
                comb['model_name'],
                comb['context_type'],
                comb['prompt_type'],
                comb['mode']
            ): comb
            for comb in combs
        }
        with alive_bar(len(future_to_stem)) as bar:
            for future in as_completed(future_to_stem):
                comb = future_to_stem[future]
                try:
                    result = future.result()
                    if result is not True:
                        print(f'error in processing {comb['id']}-{comb['model_name']}-{comb['context_type']}-{comb['prompt_type']}-{comb['mode']}; result is {result}')
                except Exception as e:
                    print(f'error in processing {comb['id']}-{comb['model_name']}-{comb['context_type']}-{comb['prompt_type']}-{comb['mode']}; exception: {e}')
                bar()


def get_targets(ids, model_names, context_types, prompt_types, tests, modes):
    targets = []
    for id in ids:
        directory = Path('data') / str(id)
        for model_name in model_names:
            for context_type in context_types:
                for prompt_type in prompt_types:
                    for test_type in tests:
                        for mode in modes:
                            targets.append((id, model_name, context_type, prompt_type, test_type, mode, ["/bin/bash", (directory / f"{test_type}_{model_name}_filled_code_{context_type}_{prompt_type}_{mode}.sh").absolute()]))
    return targets


class ParseException(Exception):
    pass


def remove_ansi(text):
    ansi_escape = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape.sub('', text)


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


def parse_unittest_libxml2(stdout, result):
    # libxml2 is weird, doesn't contain a status for passing tests.
    # Failure is indicated by a list of failing tests after a "## {NAME}" line.
    # Technically, the "## {NAME}" is the name of a group of unit tests
    # but we treat NAME as a single unit test since it doesn't list the 
    # component unit tests that pass.
    # The failure line after "## {NAME}" is something like:
    # ./test/valid/781333.xml:4: element a: validity error
    # A passing test should just list the next "## {NAME}" line or state the 
    # total, like "Total 9 tests, no errors"
    re_all = r'^## (?P<name>.*)$'
    re_failing = r'^## (?P<name>.*)\n.*error : '  # fail

    all_tests = set()
    all_matches = re.finditer(re_all, stdout, re.MULTILINE)
    for match in all_matches:
        all_tests.add(match.group("name"))
    
    failing_tests = set()
    failing_matches = re.finditer(re_failing, stdout, re.MULTILINE)
    for match in failing_matches:
        failing_tests.add(match.group("name"))
    
    passing_tests = all_tests - failing_tests

    result["pass"] = list(passing_tests)
    result["fail"] = list(failing_tests)
    result["total"] = len(all_tests)


def parse_unittest_htslib(stdout, result):
    result["total"] = 0
    pattern = unittest_patterns['htslib']
    for match in re.finditer(pattern, stdout):
        name = match.group("name")
        num_unexpected_failures = match.group("num_unexpected_failures")
        if num_unexpected_failures == '0':
            result["pass"].append(name)
        else:
            result["fail"].append(name)
        result["total"] += 1
    return result


def parse_unittest(output, project_name):
    project_name = project_name.lower()

    local_id, patch, test_type, proc = output
    stderr = proc.stderr.decode(errors='ignore')
    stdout = proc.stdout.decode(errors='ignore')

    # remove ansi escape
    stderr = remove_ansi(stderr)
    stdout = remove_ansi(stdout)

    result = {
        "pass":  [], # list of str
        "fail":  [], # list of strs or int
        "skip":  [], # list of strs or int
        "total": None  # int
    }

    # libxml2 stdout is weird, handle as special case
    if project_name == 'libxml2':
        parse_unittest_libxml2(stdout, result)
        return result

    # htslib is also weird, handle as special case
    if project_name == 'htslib':
        parse_unittest_htslib(stdout, result)
        return result

    if not project_name in unittest_patterns:
        raise ParseException(f"no pattern for {project_name}")

    patterns = unittest_patterns[project_name]
    if not isinstance(patterns, list):
        patterns = [patterns]

    for pattern in patterns:
        for test in re.finditer(pattern, stdout):
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
                            s = "pass" if s in ["ok", "okay", "success", ".", "", "done", "passed"] else s
                            s = "fail" if s in ["error", "e", "f", "fail", "not ok", "failed", "failure"] else s
                            s = "skip" if s in ["?", "skipped"] else s
                        else: # otherwise, the default status is pass
                            s = "pass"
                        for status in ["pass", "fail", "skip"]:
                            if status in s:
                                if result[status] == None:
                                    result[status] = []
                                if test.group("name") not in result[status]:
                                    result[status].append(test.group("name"))

    if result["total"] == None:
        result["total"] = sum([len(result[s]) if isinstance(result[s], list) else result[s] for s in ["pass", "fail", "skip"]])
    return result


def parse_output(output, project_name, report):
    id, model_name, context_type, prompt_type, test_type, mode, proc_data = output
    
    if test_type == "testcase":
        try:
            result = parse_testcase((id, 'sec', test_type, type('Proc', (), proc_data)()))
        except ParseException as e:
            result = "error: " + str(e)
        
    elif test_type == "unittest":
        try:
            result = parse_unittest((id, 'sec', test_type, type('Proc', (), proc_data)()), project_name)
        except ParseException as e:
            result = "error: " + str(e)

    report[id][model_name][context_type][prompt_type][mode][test_type] = result

    return report


def write_output(output):
    id, model_name, context_type, prompt_type, test_type, mode, proc = output
    directory = Path('data') / str(id) / f"{model_name}_{context_type}_{prompt_type}_{test_type}_{mode}"
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


def read_limited_output(proc, timeout=3000):
    # 50 MB limit -- so that our disk space isn't filled up, 
    # and passing cases should be much less than this. 
    # Truncated failing cases will still fail.
    max_output = 50 * 1024 * 1024
    TRUNCATION_NOTICE = b"\n[output truncated]\n"

    selector = selectors.DefaultSelector()
    stdout_chunks, stderr_chunks = [], []
    stdout_total, stderr_total = 0, 0
    stdout_truncated, stderr_truncated = False, False

    selector.register(proc.stdout, selectors.EVENT_READ)
    selector.register(proc.stderr, selectors.EVENT_READ)

    start = time.time()
    time_elapsed = 0

    while selector.get_map() and (time_elapsed < timeout) and not (stdout_truncated and stderr_truncated):
        for key, _ in selector.select(timeout=1):
            data = key.fileobj.read1(4096)  # non-blocking chunk read
            if not data:
                selector.unregister(key.fileobj)
                continue

            if key.fileobj is proc.stdout:
                if stdout_total < max_output:
                    chunk = data[:max_output - stdout_total]
                    stdout_chunks.append(chunk)
                    stdout_total += len(chunk)
                    if stdout_total >= max_output:
                        stdout_truncated = True
                else:
                    stdout_truncated = True

            elif key.fileobj is proc.stderr:
                if stderr_total < max_output:
                    chunk = data[:max_output - stderr_total]
                    stderr_chunks.append(chunk)
                    stderr_total += len(chunk)
                    if stderr_total >= max_output:
                        stderr_truncated = True
                else:
                    stderr_truncated = True
        
        time_elapsed = time.time() - start

    # Ensure process is killed if still running
    proc.kill()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        pass

    if stdout_truncated:
        stdout_chunks.append(TRUNCATION_NOTICE)
    if stderr_truncated:
        stderr_chunks.append(TRUNCATION_NOTICE)
    
    # we stopped early if process timed out or stdout/stderr filled up
    timed_out = time_elapsed >= timeout
    filled_up = stdout_truncated and stderr_truncated

    return b''.join(stdout_chunks), b''.join(stderr_chunks), timed_out, filled_up


def proc_runner(target):
    id, model_name, context_type, prompt_type, test_type, mode, cmd = target
    print(f"Running {id}_{model_name}_{context_type}_{prompt_type}_{mode}_{test_type}", flush=True)

    # get unique container id
    container_id = f"{id}_{model_name}_{context_type}_{prompt_type}_{mode}_{test_type}"

    # If no cache, cached result was a rate limit error, or rerun is True, run the process with retry
    max_retries = 5
    base_delay = 60  # 1 minute
    for attempt in range(max_retries):
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

        stdout, stderr, timed_out, filled_up = read_limited_output(proc, timeout=3000)

        return_code = proc.returncode
        if timed_out or filled_up:
            return_code = -1

            if timed_out:
                print(f"Timeout: {id}_{model_name}_{context_type}_{prompt_type}_{mode}_{test_type}", flush=True)
                stderr = b"Timeout\n" + stderr
            if filled_up:
                print(f"Truncated: {id}_{model_name}_{context_type}_{prompt_type}_{mode}_{test_type}", flush=True)
                stderr = b"Truncated\n" + stderr

            stdout = stdout or b""
            stderr = stderr or b""

        if "429 Too Many Requests" not in stderr.decode(errors='ignore'):
            break
        if attempt < max_retries - 1:
            delay = base_delay * (2 ** attempt) + random.uniform(0, 10)
            print(f"Rate limit reached. Retrying in {delay:.2f} seconds...", flush=True)
            time.sleep(delay)
        else:
            print(f"Failed to run {id}_{model_name}_{context_type}_{prompt_type}_{mode}_{test_type} after {max_retries} attempts", flush=True)

    # Always attempt to remove the container, just to be sure it's gone
    subprocess.run(['docker', 'rm', '-f', container_id], 
                   stdout=subprocess.DEVNULL, 
                   stderr=subprocess.DEVNULL)

    output = id, model_name, context_type, prompt_type, test_type, mode, {
        "stdout": stdout,
        "stderr": stderr,
        "returncode": return_code
    }

    # write to output files
    write_output(output)

    print(f"Finished {id}_{model_name}_{context_type}_{prompt_type}_{mode}_{test_type}", flush=True)
    return output


def get_remaining(targets, completed):
    return [target for target in targets if (target[0], target[1], target[2], target[3], target[4], target[5]) not in completed]


def eval(ids, model_names, prompt_types, context_types, modes, rerun, num_workers):
    # consider exposing
    tests = ['testcase', 'unittest']

    # get targes
    targets = get_targets(ids, model_names, context_types, prompt_types, tests, modes)

    # Get completed runs from existing eval report
    completed = set()
    all_report_file = Path("report_eval.json")
    if all_report_file.exists() and not rerun:
        with open(all_report_file, 'r') as f:
            all_report = json.load(f)
        for id in all_report.keys():
            for model in all_report[id].keys():
                for context in all_report[id][model].keys():
                    for prompt in all_report[id][model][context].keys():
                        for mode in all_report[id][model][context][prompt].keys():
                            for test in all_report[id][model][context][prompt][mode].keys():
                                completed.add((id, model, context, prompt, test, mode))
    # Remove non-relevant completed
    targets_completed_format = {target[:6] for target in targets}
    completed = completed.intersection(targets_completed_format)
    remaining_targets = get_remaining(targets, completed) if not rerun else targets

    print(f"Found {len(completed)} cached results cases out of {len(targets)} total targets.")
    if rerun:
        print(f"Rerunning all {len(targets)} targets, including those with cached results.")
    else:
        print(f"Running {len(remaining_targets)} targets which do not have cached results.")

    procs = []
    pool_size = min(num_workers, len(remaining_targets))
    if pool_size > 0:
        try:
            with alive_bar(len(remaining_targets)) as bar, Pool(pool_size) as p:
                for proc in p.imap_unordered(proc_runner, remaining_targets):
                    procs.append(proc)
                    bar()
        except KeyboardInterrupt:
            print("Interrupted. Processing all results...")

    # Process all targets, including cached ones
    report = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))))
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")  # Format the current date and time
    new_report_file = f"report_eval_{timestamp}.json"

    # load in sample_metadata.json
    sample_metadata = json.load(open('sample_metadata.json', "r"))
    
    if not rerun:
        # useful when parse_output has changed, and we already have the stdout and stderr
        for complete in completed:
            id, model_name, context_type, prompt_type, test_type, mode = complete
            test_patch = f"{model_name}_{context_type}_{prompt_type}_{test_type}_{mode}"

            cache_file = f'data/{id}/{test_patch}/cache.pkl'
            with open(cache_file, 'rb') as f:
                cache = pickle.load(f)

            stdout = cache['stdout']
            stderr = cache['stderr']
            returncode = cache['returncode']

            output = (id, model_name, context_type, prompt_type, test_type, mode, {
                "stdout": stdout,
                "stderr": stderr,
                "returncode": returncode
            })

            parse_output(output, sample_metadata[str(id)]["project_name"], report=report)

    with alive_bar(len(targets)) as bar:
        for target in targets:
            id, model_name, context_type, prompt_type, test_type, mode, _ = target
            output = next((p for p in procs if p[:6] == (id, model_name, context_type, prompt_type, test_type, mode)), None)
            
            if output:
                parse_output(output, sample_metadata[str(id)]["project_name"], report=report)

            bar()

    with open(new_report_file, 'w') as f:
        json.dump(report, f, indent=4)

    return new_report_file
