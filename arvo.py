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
from concurrent.futures import ProcessPoolExecutor, as_completed
import multiprocessing
import os
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


def setup(local_id, project_name, patch_path, diff, fixing_commit, root="."):

    directory = Path(root) / str(local_id)
    testcase_sec_dir = directory / "testcase_sec.sh"
    testcase_vul_dir = directory / "testcase_vul.sh"
    unittest_sec_dir = directory / "unittest_sec.sh"
    unittest_sec_print_dir = directory / "unittest_sec_print.sh"
    vul_dir = directory / "patches" / "vul.txt"
    vul_sec_base_dir = directory / "patches" / "vul_sec_base.txt"
    sec_dir = directory / "patches" / "sec.txt"
    sec_print_dir = directory / "patches" / "sec_print.txt"
    diff_dir = directory / "diff.txt"

    # only pull docker image if we don't already have it
    image_name = f"n132/arvo:{local_id}-fix"

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

    # write testcase, unittest bash scripts
    scripts_content_sec_testcase = (
        f"#!/bin/bash\n"
        "docker run --rm --init "
        f"--name {local_id}_testcase_sec "
        "--cpus=2 "
        "-e MAKEFLAGS=\"-j3\" "
        f"-v /data/oss-fuzz-bench/{local_id}/patches:/patches "
        f"n132/arvo:{local_id}-fix /bin/sh -c \" \n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:\\$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"cdilgren@umd.edu\\\"\n"
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
        # move patch file
        f"  cp -f /patches/sec.txt \\$GIT_DIR/{patch_path}\n"
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
        # Note: arvo run can give inconsistent answers sometimes (crash sometimes, pass sometimes), not sure how to handle this
        "  arvo run\n"
        "  \""
    )

    scripts_content_vul_testcase = (
        f"#!/bin/bash\n"
        "docker run --rm --init "
        f"--name {local_id}_testcase_vul "
        "--cpus=2 "
        "-e MAKEFLAGS=\"-j3\" "
        f"-v /data/oss-fuzz-bench/{local_id}/patches:/patches "
        f"n132/arvo:{local_id}-fix /bin/sh -c \" \n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:\\$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"cdilgren@umd.edu\\\"\n"
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
        # move patch file
        f"  cp -f /patches/vul_sec_base.txt \\$GIT_DIR/{patch_path}\n"
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
        # Note: arvo run can give inconsistent answers sometimes (crash sometimes, pass sometimes), not sure how to handle this
        "  arvo run\n"
        "  \""
    )

    scripts_content_sec_unittest = (
        f"#!/bin/bash\n"
        "docker run --rm --init "
        f"--name {local_id}_unittest_sec "
        "--cpus=2 "
        "-e MAKEFLAGS=\"-j3\" "
        f"-v /data/oss-fuzz-bench/{local_id}/patches:/patches "
        f"n132/arvo:{local_id}-fix /bin/sh -c \" \n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:\\$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"cdilgren@umd.edu\\\"\n"
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
        # move patch file
        f"  cp -f /patches/sec.txt \\$GIT_DIR/{patch_path}\n"
        "  " + (unittest_commands[project_name.lower()] if project_name in unittest_commands else "  echo 'NO UNIT TESTS'") + "\n"
        "  \""
    )

    scripts_content_sec_print_unittest = (
        f"#!/bin/bash\n"
        "docker run --rm --init "
        f"--name {local_id}_unittest_sec_print "
        "--cpus=2 "
        "-e MAKEFLAGS=\"-j3\" "
        f"-v /data/oss-fuzz-bench/{local_id}/patches:/patches "
        f"n132/arvo:{local_id}-fix /bin/sh -c \" \n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:\\$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"cdilgren@umd.edu\\\"\n"
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
        # move patch file
        f"  cp -f /patches/sec_print.txt \\$GIT_DIR/{patch_path}\n"
        "  " + (unittest_commands[project_name.lower()] if project_name in unittest_commands else "  echo 'NO UNIT TESTS'") + "\n"
        "  \""
    )

    # get sec, vul content from descriptions
    sec_file = get_c_cpp_file(f'descriptions/{local_id}/sec_base')
    vul_file = get_c_cpp_file(f'descriptions/{local_id}/vul_base.c')
    vul_sec_base_file = get_c_cpp_file(f'descriptions/{local_id}/vul_sec_base_base.c')
    sec_print_file = get_c_cpp_file(f'descriptions/{local_id}/sec_print_base.c')

    with open(sec_file, 'r') as f:
        sec_content = f.read()
    with open(vul_file, 'r') as f:
        vul_content = f.read()
    with open(vul_sec_base_file, 'r') as f:
        vul_sec_base_content = f.read()
    with open(sec_print_file, 'r') as f:
        sec_print_content = f.read()

    directory.mkdir(exist_ok=True, parents=True)
    (directory / "patches").mkdir(exist_ok=True)
    testcase_sec_dir.open("w").write(scripts_content_sec_testcase)
    testcase_vul_dir.open("w").write(scripts_content_vul_testcase)
    unittest_sec_dir.open("w").write(scripts_content_sec_unittest)
    unittest_sec_print_dir.open("w").write(scripts_content_sec_print_unittest)
    vul_dir.open("w").write(vul_content)
    vul_sec_base_dir.open("w").write(vul_sec_base_content)
    sec_dir.open("w").write(sec_content)
    sec_print_dir.open("w").write(sec_print_content)
    with open(diff_dir, 'w') as f:
        json.dump(diff, f, indent=4)

    subprocess.run(["chmod", "-R",  "777", str(directory.absolute())])
    return True

def get_targets(local_id, filter_patches, tests, root="./"):

    directory = Path(root) / str(local_id)
    patches = [p.stem for p in (directory / "patches").glob("./*.txt") if p.stem in filter_patches]
    targets = []

    # test whether the secure version does not crash given triggering input
    if 'testcase' in tests and 'sec' in patches:
        targets.append((local_id, 'sec', "testcase", ["/bin/bash", (directory / "testcase_sec.sh").absolute()]))

    # test whether the vulnerable version does crash given triggering input
    if 'testcase' in tests and 'vul' in patches:
        targets.append((local_id, 'vul', "testcase", ["/bin/bash", (directory / "testcase_vul.sh").absolute()]))

    # test whether there are passing unit tests for the secure version
    if 'unittest' in tests and 'sec' in patches:
        targets.append((local_id, 'sec', "unittest", ["/bin/bash", (directory / "unittest_sec.sh").absolute()]))

    # test whether there are relevant unit tests for the secure version
    if 'unittest' in tests and 'sec_print' in patches:
        targets.append((local_id, 'sec_print', "unittest", ["/bin/bash", (directory / "unittest_sec_print.sh").absolute()]))

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
    re_failing = r'^## (?P<name>.*)(?=\n\.)'  # fail

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
        for test in re.finditer(pattern, stdout + "\n" + stderr):
            if test.group("name") == "error_value_test":
                pass
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
        report[str(local_id)][f"{test_type}_{patch}"] = result
    else:
        report[str(local_id)] = {f"{test_type}_{patch}": result}

    return report

def print_report(report):
    for local_id, test_results in report.items():
        print(local_id)

        if 'testcase_sec' in test_results:
            print(f'testcase_sec\t{test_results['testcase_sec']}')
        if 'testcase_vul' in test_results:
            print(f'testcase_vul\t{test_results['testcase_vul']}')
        if 'unittest_sec' in test_results:
            num_pass = len(test_results['unittest_sec']['pass'])
            num_total = test_results['unittest_sec']['total']
            print(f'unittest_sec\t{num_pass}/{num_total}')
        if 'unittest_sec_print' in test_results:
            num_pass = len(test_results['unittest_sec_print']['pass'])
            num_total = test_results['unittest_sec_print']['total']
            print(f'unittest_sec_print\t{num_pass}/{num_total}')

def write_output(output, root="./"):
    local_id, patch, test_type, proc = output
    directory = Path(root) / str(local_id) / f"{test_type}_{patch}"
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

def proc_runner(target_with_output_path_and_rerun):
    target, output_path, rerun = target_with_output_path_and_rerun
    local_id, patch, test_type, cmd = target
    print(f"Running {local_id} {patch} {test_type}")
   
    # get unique container id
    container_id = f"{local_id}_{test_type}_{patch}"

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
                print(f"Timeout: {local_id}_{test_type}_{patch}", flush=True)
                stderr = b"Timeout\n" + stderr
            if filled_up:
                print(f"Truncated: {local_id}_{test_type}_{patch}", flush=True)
                stderr = b"Truncated\n" + stderr

            stdout = stdout or b""
            stderr = stderr or b""

        if "429 Too Many Requests" not in stderr.decode(errors='ignore'):
            break
        if attempt < max_retries - 1:
            delay = base_delay * (2 ** attempt) + random.uniform(0, 10)
            print(f"Rate limit reached. Retrying in {delay:.2f} seconds...")
            time.sleep(delay)
    else:
        print(f"Failed to run {local_id} {patch} {test_type} after {max_retries} attempts")

    # Always attempt to remove the container, just to be sure it's gone
    subprocess.run(['docker', 'rm', '-f', container_id], 
                   stdout=subprocess.DEVNULL, 
                   stderr=subprocess.DEVNULL)

    output = local_id, patch, test_type, {
        "stdout": stdout,
        "stderr": stderr,
        "returncode": return_code
    }

    # write to output files
    write_output(output, output_path)

    print(f"Finished {local_id} {patch} {test_type}")
    return output

def get_remaining(targets, completed):
    return [target for target in targets if (target[0], f"{target[2]}_{target[1]}") not in completed]

def main():
    cases_fname = "filter_logs/cases.json"

    # Define the set of predefined actions
    actions = ['load', 'setup', 'eval']
    parser = argparse.ArgumentParser(description="A program that performs actions on targets.")
    parser.add_argument('action', choices=actions, help="The action to perform. Must be one of: " + ", ".join(actions))
    parser.add_argument('targets', nargs='+', help='Targets as one or more integers or the string "all".')
    parser.add_argument("-p", "--path", type=str, help="", default="./")
    parser.add_argument("-o", "--output", type=str, help="", default="./")
    parser.add_argument("-f", "--filter-patches", nargs='+', help="enter 'sec', 'vul', and/or 'sec_print")
    parser.add_argument("--rerun", action="store_true", help="Rerun targets even if they have cached results")
    parser.add_argument("--tests", nargs='+', help="enter 'testcase' and/or 'unittest'")

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
                targets = [str(int(t)) for t in args.targets]
            except ValueError:
                parser.error('Targets must be integers or "all".')

        if args.action == "setup":

            num_workers = min(12, len(targets))

            with ProcessPoolExecutor(max_workers=num_workers) as executor:
                future_to_stem = {
                    executor.submit(setup, local_id, data[local_id]["project_name"], data[local_id]["changed_file"], data[local_id]["diff"], data[local_id]["fixing_commit"], root=root): local_id
                    for local_id in targets
                }

                with alive_bar(len(future_to_stem)) as bar:
                    for future in as_completed(future_to_stem):
                        local_id = future_to_stem[future]

                        try:
                            result = future.result()
                            if result is not True:
                                print(f'error in processing {local_id}; result is {result}')
                        except Exception as e:
                            print(f'error in processing {local_id}; exception: {e}')

                        bar()

        if args.action == "eval":
            targets = [c for id in targets for c in get_targets(id, args.filter_patches, args.tests, root=root)]
            
            # Get completed runs from existing report
            completed = set()
            all_report_file = Path(args.output) / f"report.json"
            if all_report_file.exists() and not args.rerun:
                with open(all_report_file, 'r') as f:
                    all_report = json.load(f)
                for id in all_report.keys():
                    for test_patch in all_report[id].keys():
                        completed.add((id, test_patch))
            # Remove non-relevant completed
            targets_comp_format = {(target[0], f"{target[2]}_{target[1]}") for target in targets}
            completed = completed.intersection(targets_comp_format)

            # Count cached results and identify rate-limited cases
            cached_count = 0
            rate_limited_count = 0
            time_out_count = 0
            for target in targets:
                local_id, patch, test_type, _ = target
                cache_file = Path(args.output) / str(local_id) / f"{test_type}_{patch}" / "cache.pkl"
                if cache_file.exists() and not args.rerun:
                    try:
                        with cache_file.open("rb") as f:
                            cached_data = pickle.load(f)
                        stderr = cached_data.get('stderr', b'').decode(errors="ignore")
                        if "429 Too Many Requests" in stderr:
                            rate_limited_count += 1
                            # Remove from completed set to ensure it's rerun
                            completed.discard((local_id, f"{test_type}_{patch}"))
                        elif stderr.startswith("Timeout") or stderr.startswith("Truncated") or stderr.startswith("docker: "):
                            time_out_count += 1
                            completed.discard((local_id, f"{test_type}_{patch}"))
                        else:
                            cached_count += 1
                    except:
                        pass

            print(f"Found {cached_count} valid cached results, {rate_limited_count} rate-limited cases, and {time_out_count} time out cases out of {len(targets)} total targets.")
            if args.rerun:
                print("Rerunning all targets, including those with cached results.")

            procs = []
            pool_size = min(4, len(targets))
            try:
                with alive_bar(len(targets)) as bar, Pool(pool_size) as p:
                    remaining_targets = get_remaining(targets, completed) if not args.rerun else targets
                    targets_with_output = [(target, args.output, args.rerun) for target in remaining_targets]
                    for proc in p.imap_unordered(proc_runner, targets_with_output):
                        procs.append(proc)
                        bar()

            except KeyboardInterrupt:
                print("Interrupted. Processing all results...")

            # Process all targets, including cached ones
            report = {}
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")  # Format the current date and time
            new_report_file = Path(args.output) / f"report_{timestamp}.json"

            if not args.rerun:
                # useful when parse_output has changed, and we already have the stdout and stderr
                for complete in completed:
                    local_id, test_patch = complete
                    test_type = test_patch.split('_')[0]
                    patch = test_patch.replace(f'{test_type}_', '')

                    cache_file = f'/data/oss-fuzz-bench/output/{local_id}/{test_patch}/cache.pkl'
                    with open(cache_file, 'rb') as f:
                        cache = pickle.load(f)

                    stdout = cache['stdout']
                    stderr = cache['stderr']
                    returncode = cache['returncode']

                    output = (local_id, patch, test_type, {
                        "stdout": stdout,
                        "stderr": stderr,
                        "returncode": returncode
                    })

                    parse_output(output, data[str(local_id)]["project_name"], report=report)

            with alive_bar(len(targets)) as bar:
                for target in targets:
                    local_id, patch, test_type, _ = target
                    output = next((p for p in procs if p[:3] == (local_id, patch, test_type)), None)
                    
                    if output:
                        parse_output(output, data[str(local_id)]["project_name"], report=report)

                    bar()
            
            json.dump(report, new_report_file.open("w"), indent=4)

            print_report(report)

if __name__ == "__main__":
    main()