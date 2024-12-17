import sys
import os
import re
import csv
import json
import lizard
from utils import *
from filter import make_mangled_name
from tree_sitter import Language, Parser


def remove_comments(input_string):
    # Pattern to match single-line comments (//) and multi-line comments (/* */)
    comment_pattern = re.compile(r'(//.*?$|/\*.*?\*/)', re.DOTALL | re.MULTILINE)
    
    # Replace comments with an empty string
    result = re.sub(comment_pattern, '', input_string)
    
    return result


def find_functions_and_declarations(node, source_code, func_pattern, recursion_level=1):
    """Recursively finds function definitions and their variable declarations."""
    recursion_level += 1
    results = []

    if 286 <= node.start_point[0] and node.start_point[0] <= 289:
        pass

    if node.type == 'function_definition' or node.type == 'compound_statement' or node.type == 'labeled_statement':
        text = node.text.decode('utf-8').replace('\n', '').replace('\t', '').replace('void', '')
        text = remove_comments(text)
        if re.search(func_pattern, text) is not None:
            # Locate variable declarations within the function body
            declarations = []
            # find the compound statment
            for child in node.children:
                if child.type == 'compound_statement':
                    # get any declarations in the compound statement
                    for child_cmp in child.children:
                        if child_cmp.type == 'declaration':
                            declarations.append(child_cmp)
                    break
            results.append((child, declarations))

    for child in node.children:
        if recursion_level < 985:
            results.extend(find_functions_and_declarations(child, source_code, func_pattern, recursion_level))
    return results


def find_closest_func_dec(func, funcs_decs):
    # possible that multiple structs/classes have the same function
    # heuristic: use func's start and end lines
    min_dist = None
    for func_dec in funcs_decs:
        dist = abs(func_dec[0].start_point[0] - func.start_line)
        dist += abs(func_dec[0].end_point[0] - func.end_line)

        if min_dist is None or dist < min_dist:
            min_dist = dist
            min_func_dec = func_dec

    return [min_func_dec]


def insert_after_open_bracket(mod_func, source_code_lines):
    # go to function start (line right after the '{')
    # TODO: handle functions with one line, which do not need brackets
    line_number = mod_func.start_line - 1
    line = source_code_lines[line_number]
    while '{' not in line:
        line_number += 1
        line = source_code_lines[line_number]
    line_number += 1

    indent = " " * (len(source_code_lines[line_number + 1]) - len(source_code_lines[line_number + 1].lstrip()))

    return line_number, indent


def main(id):
    print(f"Processing {id}")
    cases_file = f'filter_logs_all/cases.json'
    source_file = f'/data/cmd-oss-fuzz-bench/{id}/patches/vul.txt'
    destination_file = f'/data/cmd-oss-fuzz-bench/{id}/patches/vul_print.txt'

    # Get the modified file name from cases file
    id = str(id)
    with open(cases_file, 'r') as f:
        cases = json.load(f)
    file_name = cases[id]['changed_file']
    if file_name is None:
        print(f"Could not find file name in cases file for id {id}")
        return

    # Determine the language based on file extension
    ext = get_file_extension(file_name)
    language = determine_language(ext)
    if language == 'c':
        LANGUAGE = C_LANGUAGE
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
    else:
        print(f"Language of modified file not recognized for id {id}")
        return

    # Read the modified source code -- use regex for \n to ignore special characters like FF \x0c
    with open(source_file, 'r') as f:
        source_code = f.read()
        source_code_lines = re.split(r'\n', source_code)

    # Find the function containing the first modified line
    diff = cases[id]['diff']
    mod_lines = [d[0] for d in diff['deleted']] + [d[0] for d in diff['added']]
    file_lizard_src = lizard.analyze_file.analyze_source_code(file_name, source_code)
    for func in file_lizard_src.function_list:
        for line_num in mod_lines:
            if func.start_line <= line_num <= func.end_line:
                mod_func = func

    # changed_function = cases[id]['changed_function']
    # file_lizard_src = lizard.analyze_file.analyze_source_code(file_name, source_code)
    # func_mangled_names = [make_mangled_name(func.name, func.full_parameters) for func in file_lizard_src.function_list]
    # func_i = func_mangled_names.index(changed_function)
    # func = file_lizard_src.function_list[func_i]

    # Parse the source code with Tree-sitter
    parser = Parser()
    parser.set_language(LANGUAGE)
    tree = parser.parse(bytes(source_code, 'utf8'))
    root_node = tree.root_node

    # get declarations in the function
    func_pattern = mod_func.long_name.split('::')[-1].replace('*', r'\*').replace('(', r'\s*\(').replace(')', r'\s*\)').replace('[', r'\s*\[').replace(']', r'\s*\]').replace('&', r'\s*&\s*').replace(',', r'\s*,\s*').replace(' ', r'\s*').replace('void', '') + r'\s*[\s\w:\(\),]*{'  # + r'\s*{'
    funcs_decs = find_functions_and_declarations(root_node, source_code, func_pattern)
    if len(funcs_decs) == 0:
        print(f"Tree sitter failed to find function for {id}, inserting after open bracket")
        line_number, indent = insert_after_open_bracket(mod_func, source_code_lines)
    elif len(funcs_decs) > 1:
        funcs_decs = find_closest_func_dec(mod_func, funcs_decs)
        # find line after first declaration or after first child
        if len(funcs_decs[0][1]) > 0:
            node = funcs_decs[0][1][-1]
            line_number = node.end_point[0] + 1
            indent = " " * (len(source_code_lines[line_number - 1]) - len(source_code_lines[line_number - 1].lstrip()))
        else:  # no var declarations in this function
            node = funcs_decs[0][0].children[0]  # this is the '{'
            line_number = node.end_point[0] + 1
            indent = " " * (len(source_code_lines[line_number]) - len(source_code_lines[line_number].lstrip()))
    else: 
        # find line after first declaration or after first child
        if len(funcs_decs[0][1]) > 0:
            node = funcs_decs[0][1][-1]
            line_number = node.end_point[0] + 1
            indent = " " * (len(source_code_lines[line_number - 1]) - len(source_code_lines[line_number - 1].lstrip()))
        else:  # no var declarations in this function
            node = funcs_decs[0][0].children[0]  # this is the '{'
            line_number = node.end_point[0] + 1
            indent = " " * (len(source_code_lines[line_number]) - len(source_code_lines[line_number].lstrip()))

    # insert print statement
    source_code_lines.insert(line_number, f'{indent}printf("This is a test for CodeGuard+\\n");')

    print(f'\n***************** ID {id} *************************')
    for ln in source_code_lines[line_number-5:line_number+5]:
        print(ln)
    print('\n\n')
    
    # Replace the function in the source code
    modified_source_code = '\n'.join(source_code_lines)
    with open(destination_file, 'w') as f:
        f.write(modified_source_code)

if __name__ == '__main__':
    with open('filter_logs_all/testable_ids.txt', 'r') as f:
        ids = f.read().splitlines()
    # ids = sorted([int(id) for id in ids])
    for id in ids:
        main(id)
