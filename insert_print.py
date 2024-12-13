import sys
import os
import re
import csv
import json
import lizard
from utils import *
from filter import make_mangled_name

def main(id):
    print(f"Processing {id}")
    cases_file = f'filter_logs_all/cases.json'
    source_file = f'/data/cmd-oss-fuzz-bench-ghostpdl/{id}/patches/vul.txt'
    destination_file = f'/data/cmd-oss-fuzz-bench-ghostpdl/{id}/patches/vul_print.txt'

    # Get the modified file name, diff, and modified function from cases file
    id = str(id)
    with open(cases_file, 'r') as f:
        cases = json.load(f)
    file_name = cases[id]['changed_file']
    if file_name is None:
        print("Could not find file name in cases file")
        sys.exit(1)

    diff = cases[id]['diff']
    if len(diff['added']) == 0 and len(diff['deleted']) == 0:
        print("No additions or deletions found in diff file")
        sys.exit(1)

    changed_function = cases[id]['changed_function']

    # Read the modified source code
    with open(source_file, 'r') as f:
        source_code = f.read()
        source_code_lines = source_code.splitlines()

    # Find the function containing the first modified line
    file_lizard_src = lizard.analyze_file.analyze_source_code(file_name, source_code)
    func_mangled_names = [make_mangled_name(func.name, func.full_parameters) for func in file_lizard_src.function_list]
    func_i = func_mangled_names.index(changed_function)
    func = file_lizard_src.function_list[func_i]

    # go to function start (line right after the '{')
    # TODO: handle functions with one line, which do not need brackets
    line_num = func.start_line - 1
    line = source_code_lines[line_num]
    while '{' not in line:
        line_num += 1
        line = source_code_lines[line_num]
    line_num += 1
    line = source_code_lines[line_num]

    # go to after declarations
    declaration_pattern = r"(?P<type>[a-zA-Z_][a-zA-Z0-9_]*(?:\s+\*+|\s+))(?P<name>[a-zA-Z_][a-zA-Z0-9_]*)(\s*\[\s*\d*\s*\])*(\s*=\s*[^;,]*)?\s*;"
    while re.search(declaration_pattern, line) is not None or line.strip() == '':
        line_num += 1
        line = source_code_lines[line_num]

    # insert print statement
    mod_source_code_lines = source_code_lines[:line_num] + ['printf("This is a test for CodeGuard+\\n");'] + source_code_lines[line_num:]
    
    # Replace the function in the source code
    modified_source_code = '\n'.join(mod_source_code_lines)
    with open(destination_file, 'w') as f:
        f.write(modified_source_code)

if __name__ == '__main__':
    with open('ids_print_ex_no_header.csv', 'r') as f:
        ids = f.read().splitlines()
    ids = sorted([int(id) for id in ids])
    for id in ids:
        main(id)