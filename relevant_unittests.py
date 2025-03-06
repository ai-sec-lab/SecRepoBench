import json
import re

from projects import *
test_before_projects = ['ffmpeg', 'file', 'c-blosc2', 'fluent-bit', 'assimp', 'php-src', 'libxml2', 'imagemagick']
no_colon_projects = ['harfbuzz', 'libplist', 'yara']

def get_relevant_unittests(target_project, stdout):
    test_before = target_project in test_before_projects
    no_colon = target_project in no_colon_projects
    # Pattern to match unit test results
    pattern = unittest_patterns[target_project]
    if no_colon:
        pattern = r"\n(?P<status>[A-Z]+) (?P<name>.*) \("
    if target_project == 'c-blosc2' or target_project == 'libdwarf':
        pattern = r"Test: (?P<name>.*)\n"
    elif target_project == 'fluent-bit':
        pattern = r"Test (?P<name>.*)\.\.\."
    elif target_project == 'php-src' or target_project == 'imagemagick':
        pattern = r"Test Name: (?P<name>.*)\n"
    elif target_project == 'libxml2':
        pattern = r"## (?P<name>.*)\n"
    elif target_project == 'mruby':
        pattern = r"\n(?P<name>[^\[\]]+)(\[([0-9]|\.)+\])? : "
    elif target_project == 'libsndfile':
        pattern = r'(?m)^\d+/\d+\s+(?:Testing:|Test:)\s+(?P<name>\w+)'
    
    # Find all matches with their positions
    matches = list(re.finditer(pattern, stdout))
    
    # Track tests that called the function
    relevant_unittests = []
    
    # Search for occurrences of "This is a test for CodeGuard+"
    function_call_positions = [m.start() for m in re.finditer(r"This is a test for CodeGuard\+", stdout)]

    if not function_call_positions:
        return relevant_unittests  # No function calls detected

    last_test_pos = 0
    last_test_name = None

    # Iterate through each unit test match
    # If do_before is true, we consider the first unit after the function call, rather the last one befor
    for match in matches:
        test_name = match.group("name")
        test_pos = match.start()  # Position of test match in text

        # Check if any function call happened after last test but before this one
        if any(last_test_pos < pos < test_pos for pos in function_call_positions):
            if test_before:
                if last_test_name:
                    relevant_unittests.append(last_test_name)
            else:
                relevant_unittests.append(test_name)

        # Update last test position and name
        last_test_pos = test_pos
        last_test_name = test_name

    # check after the last match
    if test_before and any(last_test_pos < pos for pos in function_call_positions):
        relevant_unittests.append(last_test_name)

    return relevant_unittests


def main():
    # Say that file's test are used in test.c
    # target_projects = ['ffmpeg', 'hunspell', 'harfbuzz', 'libxml2', 'c-blosc2', 'fluent-bit', 'gpac', 'lcms', 'assimp', 'file', 'libplist', 'libdwarf', 'yara', 'php-src']
    # target_projects = ['ndpi', 'imagemagick', 'mruby']
    target_projects = ['lcms']

    with open('/space1/cdilgren/project_benchmark/analyze_report/ids_each_step_by_proj.json', 'r') as f:
        ids_each_step_by_proj = json.load(f)

    results = {}
    for target_project in target_projects:
        print(f"Processing {target_project}")
        ids_pass_testcase_unittest = ids_each_step_by_proj['ids_pass_testcase_unittest'][target_project]

        for id in ids_pass_testcase_unittest:
            with open(f'/data/oss-fuzz-bench/output/{id}/unittest_sec_print/stdout.txt', 'rb') as f:
                stdout = f.read().decode('utf-8', errors='ignore')

            relevant_unittests = get_relevant_unittests(target_project, stdout)
            results[id] = {"relevant_unittests": relevant_unittests}

    with open('relevant_unittests.json', 'w') as f:
        json.dump(results, f, indent=4)


if __name__ == "__main__":
    main()

