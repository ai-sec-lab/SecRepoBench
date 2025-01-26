import sys
import os
import re
import csv
import tree_sitter_c as tsc
import tree_sitter_cpp as tscpp
from tree_sitter import Language, Parser
import random
import json
from utils import *

def extract_func(id, case, id2var):
    print(f"Processing {id}")

    # Determine the language based on file extension
    changed_file = case['changed_file']
    ext = get_file_extension(changed_file)
    language = determine_language(ext)
    LANGUAGE = C_LANGUAGE if language == 'c' else CPP_LANGUAGE

    # Read the modified source code
    if language == 'c':
        source_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask.c'
    else:
        source_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask.cpp'

    with open(source_file, 'r') as f:
        source_code = f.read()

    # Parse the source code with Tree-sitter
    parser = Parser(LANGUAGE)
    tree = parser.parse(bytes(source_code, 'utf8'))

    x = None
    for i, line in enumerate(source_code.splitlines()):
        if "// <MASK>" in line:
            x = i + 1

    if x is None:
        print("No masked code found in the source file.")
        sys.exit(1)

    function_node = find_function_containing_line(tree, x)  # need to get function node in same way as mask.py

    if function_node is None:
        print("No modified lines found within any function.")
        sys.exit(1)

    if '// <MASK>' not in function_node.text.decode('utf-8'):
        # print("No masked code found in the function.")
        for i in range(len(source_code.splitlines())):
            function_node = find_function_containing_line(tree, i + 1)
            if function_node is not None and '// <MASK>' in function_node.text.decode('utf-8'):
                break

    variables = find_variables(function_node)
    
    if variables:
        old_var = random.choice(variables).text.decode('utf-8')
        id2var[id] = old_var
        print(f"Variable found: {old_var}")
    else:
        id2var[id] = None
        print("No variables found in the function.")

if __name__ == "__main__":
    random.seed(42)
    with open('ids_top40.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    id2var = {}

    for id in ids:
        case = cases[id]
        extract_func(id, case, id2var)

    with open('id2var.json', 'w') as f:
        json.dump(id2var, f, indent=4)