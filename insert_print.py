import sys
import os
import re
import csv
from tree_sitter import Language, Parser
from utils import *

def main(id):
    print(f"Processing {id}")
    diff_file = f'/space1/yanjun/ethan/arvo-oss-fuzz-bench/ARVO-Meta/patches/{id}.diff'
    source_file = f'/data/fyj-oss-fuzz-bench/{id}/patches/vul.txt'
    destination_file = f'/data/fyj-oss-fuzz-bench/{id}/patches/vul_print.txt'

    # Parse the diff file to get the file name and modified lines
    file_name, modified_lines = parse_diff_file(diff_file)
    if file_name is None:
        print("Could not find file name in diff file")
        sys.exit(1)
    if not modified_lines:
        print("No additions or deletions found in diff file")
        sys.exit(1)

    # Determine the language based on file extension
    ext = get_file_extension(file_name)
    language = determine_language(ext)
    LANGUAGE = C_LANGUAGE if language == 'c' else CPP_LANGUAGE

    # Read the modified source code
    with open(source_file, 'r') as f:
        source_code = f.read()
        total_lines = len(source_code.splitlines())

    # Parse the source code with Tree-sitter
    parser = Parser()
    parser.set_language(LANGUAGE)
    tree = parser.parse(bytes(source_code, 'utf8'))

    # Find the function containing the first modified line
    x = min(modified_lines)
    y = max(modified_lines)
    # Adjust x if it exceeds total_lines
    if x > total_lines:
        x = total_lines

    modified_lines = sorted(modified_lines)
    function_node = None

    # Find the first modified line within a function
    for line_num in modified_lines:
        if line_num > total_lines:
            continue  # Skip if line number exceeds total lines
        function_node = find_function_containing_line(tree, line_num)
        if function_node is not None:
            x = line_num  # Update x to the line number within the function
            break

    function_text = function_node.text.decode('utf-8')
    assert function_text in source_code, "Function text not found in source code"
    start = function_text.find('{') + 1
    # insert print statement
    new_function_text = function_text[:start] + '\nprintf("This is a test for CodeGuard+\\n");\n' + function_text[start:]
    # Replace the function in the source code
    modified_code = source_code.replace(function_text, new_function_text)
    with open(destination_file, 'w') as f:
        f.write(modified_code)

if __name__ == '__main__':
    with open('/home/yanjun/arvo-oss-fuzz-bench/ids/testable_ids.txt', 'r') as f:
        ids = f.read().splitlines()
    ids = sorted([int(id) for id in ids])
    for id in ids:
        main(id)