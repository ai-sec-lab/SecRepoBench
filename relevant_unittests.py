import json
import re

from projects import *


def get_relevant_unittests(target_project, stdout):
    # Pattern to match unit test results
    pattern = unittest_patterns[target_project]
    
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
    for match in matches:
        test_name = match.group("name")
        test_pos = match.start()  # Position of test match in text

        # Check if any function call happened after last test but before this one
        if any(last_test_pos < pos < test_pos for pos in function_call_positions):
            relevant_unittests.append(test_name)

        # Update last test position and name
        last_test_pos = test_pos
        last_test_name = test_name

    return relevant_unittests


def main():
    target_projects = ['ffmpeg']  #['ndpi']

    with open('/home/cdilgren/project_benchmark/analyze_report/ids_each_step_by_proj.json', 'r') as f:
        ids_each_step_by_proj = json.load(f)

    for target_project in target_projects:
        ids_pass_testcase_unittest = ids_each_step_by_proj['ids_pass_testcase_unittest'][target_project]

        results = {}
        for id in ids_pass_testcase_unittest:
            with open(f'/home/cdilgren/project_benchmark/oss-fuzz-bench/output/{id}/unittest_sec_print/stdout.txt') as f:
                stdout = f.read()
            relevant_unittests = get_relevant_unittests(target_project, stdout)
            results[id] = {"relevant_unittests": relevant_unittests}

    with open('relevant_unittests.json', 'w') as f:
        json.dump(results, f, indent=4)


if __name__ == "__main__":
    main()
