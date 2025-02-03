import sys
import os
import re
import csv
import json
import lizard
from utils import *
from filter import make_mangled_name
import tree_sitter_c as tsc
import tree_sitter_cpp as tscpp
from tree_sitter import Language, Parser


C_LANGUAGE = Language(tsc.language())
CPP_LANGUAGE = Language(tscpp.language())


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

    if min_dist < 6:  # +/- 3 lines from start and end
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


def get_decs(function_node):
    """Gets a function's variable declarations."""
    declarations = []

    # find the compound statment
    for child in function_node.children:
        if child.type == 'compound_statement':
            # get any declarations in the compound statement
            for child_cmp in child.children:
                if child_cmp.type == 'declaration':
                    declarations.append(child_cmp)
            break

    return declarations


def insert_print(function_node, mod_func, source_code_lines):

    if function_node is None:
        line_number, indent = insert_after_open_bracket(mod_func, source_code_lines)

    else:
        # get declarations in the function
        declarations = get_decs(function_node)

        # find line after last declaration or after first child
        if len(declarations) > 0:
            last_dec = declarations[-1]
            line_number = last_dec.end_point[0] + 1
            indent = " " * (len(source_code_lines[line_number - 1]) - len(source_code_lines[line_number - 1].lstrip()))
        else:  # no var declarations in this function
            opening_bracket_node = find_opening_bracket(function_node)
            line_number = opening_bracket_node.end_point[0] + 1
            indent = " " * (len(source_code_lines[line_number]) - len(source_code_lines[line_number].lstrip()))

    # insert print statement
    source_code_lines.insert(line_number, f'{indent}printf("This is a test for CodeGuard+\\n");')

    # print(f'\n***************** ID {id} *************************')
    # for ln in source_code_lines[line_number-5:line_number+5]:
    #     print(ln)
    # print('\n\n')
    
    # Replace the function in the source code
    modified_source_code = '\n'.join(source_code_lines)
    return modified_source_code

if __name__ == '__main__':
    with open('/home/cdilgren/project_benchmark/ids_simple.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    for id in ids:
        insert_print(id)
    