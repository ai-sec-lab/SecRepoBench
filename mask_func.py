import sys
import os
import re
import csv
import json
import difflib
import tree_sitter_c as tsc
import tree_sitter_cpp as tscpp
from tree_sitter import Language, Parser
import lizard
from utils import *


def get_leading_whitespace(text):
    """
    Extract the leading whitespace (spaces or tabs) from a string.
    
    Args:
        text (str): The input string to analyze
        
    Returns:
        str: The leading whitespace characters found
    """
    leading = ''
    for char in text:
        if char in ' \t\n':
            leading += char
        else:
            break
    return leading


def remove_comments(input_string):
    # Pattern to match single-line comments (//) and multi-line comments (/* */)
    comment_pattern = re.compile(r'(//.*?$|/\*.*?\*/)', re.DOTALL | re.MULTILINE)
    
    # Replace comments with an empty string
    result = re.sub(comment_pattern, '', input_string)
    
    return result


def remove_sl_comments_code_block(code_block):
    code_block_lines = re.split(r'\n', code_block)
    code_block_lines = [remove_comments(line) for line in code_block_lines]
    return '\n'.join(code_block_lines)


def find_functions(node, func_pattern, x, y):
    """Recursively finds function definitions and their variable declarations."""
    results = []

    def traverse(node, x, y):
        if node.type == 'function_definition' or node.type == 'compound_statement' or node.type == 'labeled_statement':
            text = node.text.decode('utf-8').replace('\n', '').replace('\t', '').replace('void', '')
            text = remove_comments(text)
            if re.search(func_pattern, text) is not None:
                if (x >= node.start_point[0] + 1) and (y <= node.end_point[0] + 1):
                    results.append(node)
        else:
            for child in node.children:
                traverse(child, x, y)

    traverse(node, x, y)

    return results


def find_closest_func(func_lizard, funcs_ts):
    # possible that multiple structs/classes have the same function
    # heuristic: use func's start and end lines
    min_dist = None
    for func in funcs_ts:
        dist = abs(func[0].start_point[0] - func_lizard.start_line)
        dist += abs(func[0].end_point[0] - func_lizard.end_line)

        if min_dist is None or dist < min_dist:
            min_dist = dist
            min_func_dec = func

    if min_dist < 6:  # +/- 3 lines from start and end
        return [min_func_dec]


def parse_git_diff(diff_content, target_filename):
    """
    Parse a git diff file and extract added/deleted lines for a specific file.
    
    Args:
        diff_content (str): Content of the git diff file
        target_filename (str): Name of the file to extract changes for
        
    Returns:
        dict: Dictionary with 'added' and 'deleted' keys, each containing a list of
              [line_number, content] pairs
    """
    result = {
        'added': [],
        'deleted': []
    }
    
    # Split the diff into sections for different files
    sections = diff_content.split('diff --git')
    
    # Find the section for our target file
    target_section = None
    for section in sections:
        if target_filename in section.split('\n')[0]:
            target_section = section
            break
            
    if not target_section:
        return result
        
    # Parse the section
    current_line_old = 0
    current_line_new = 0
    
    lines = target_section.split('\n')
    in_hunk = False
    
    for line in lines:
        # Skip header lines
        if line.startswith('---') or line.startswith('+++') or line.startswith('index'):
            continue
            
        # Parse hunk header
        if line.startswith('@@'):
            in_hunk = True
            # Extract starting line numbers
            # Format: @@ -old_start,old_count +new_start,new_count @@
            parts = line.split(' ')
            old_start = int(parts[1].split(',')[0][1:])  # Remove the '-' prefix
            new_start = int(parts[2].split(',')[0][1:])  # Remove the '+' prefix
            current_line_old = old_start
            current_line_new = new_start
            continue
            
        if not in_hunk:
            continue
            
        # Process line changes
        if line.startswith('-'):
            result['deleted'].append([current_line_old, line[1:]])
            current_line_old += 1
        elif line.startswith('+'):
            result['added'].append([current_line_new, line[1:]])
            current_line_new += 1
        elif line:  # Context line (unchanged)
            current_line_old += 1
            current_line_new += 1
            
    return result


def mask_func(id, diff_non_trivial, changed_file, base_path):

    print(f"Processing {id}")

    with open('/home/cdilgren/project_benchmark/filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    # set paths
    diff_file = f'/home/cdilgren/project_benchmark/ARVO-Meta/patches/{id}.diff'
    desc_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/desc.txt'

    base_path_id = os.path.join(base_path, id)
    if not os.path.exists(base_path_id):
        os.mkdir(base_path_id)

    # Determine the language based on file extension
    ext = get_file_extension(changed_file)
    language = determine_language(ext)
    if language == 'c':
        LANGUAGE = C_LANGUAGE
        sec_code_block_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block.c'
        mask_func_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_func.c'
        mask_func_desc_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_func_desc.c'
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
        sec_code_block_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block.cpp'
        mask_func_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_func.cpp'
        mask_func_desc_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_func_desc.c'
    else:
        print(f"Language of modified file not recognized for id {id}")
        return

    # get sec_code_block
    with open(sec_code_block_file, 'r') as f:
        sec_code_block = f.read()
    sec_code_block_lines = re.split(r'\n', sec_code_block)

    # Read the modified source code -- use regex for \n to ignore special characters like FF \x0c
    source_code = cases[id]['source_code']
    source_code_lines = re.split(r'\n', source_code)

    # Get full diff file
    with open(diff_file, 'r') as f:
        diff_file_content = f.read()
    diff = parse_git_diff(diff_file_content, changed_file)

    # Get the modified line numbers in the source file for added and deleted
    modified_lines = [change[0] for change in diff_non_trivial['added']]

    added_lines = [change[0] for change in diff['added']]
    deleted_lines = [change[0] for change in diff['deleted']]
    deleted_nt_lines = [change[0] for change in diff_non_trivial['deleted']]

    while len(added_lines) != 0 or len(deleted_lines) != 0:
        if len(added_lines) == 0:
            line = deleted_lines.pop(0)
            if line not in modified_lines and line in deleted_nt_lines:
                deleted_nt_lines.remove(line)
                modified_lines.append(line)
            deleted_lines = [line - 1 for line in deleted_lines]
            deleted_nt_lines = [line - 1 for line in deleted_nt_lines]
        elif len(deleted_lines) == 0:
            break
        elif added_lines[0] == deleted_lines[0]:
            added_lines.pop(0)
            line = deleted_lines.pop(0)
            if line not in modified_lines and line in deleted_nt_lines:
                deleted_nt_lines.remove(line)
                modified_lines.append(line)
        elif added_lines[0] < deleted_lines[0]:
            deleted_lines = [line + 1 for line in deleted_lines]
            deleted_nt_lines = [line + 1 for line in deleted_nt_lines]
            added_lines.pop(0)
        else:
            line = deleted_lines.pop(0)
            if line not in modified_lines and line in deleted_nt_lines:
                deleted_nt_lines.remove(line)
                modified_lines.append(line)
            deleted_lines = [line - 1 for line in deleted_lines]
            deleted_nt_lines = [line - 1 for line in deleted_nt_lines]

    modified_lines = sorted(modified_lines)

    # Find the function containing the first modified line (lizard)
    mod_lines = [d[0] for d in diff_non_trivial['added']] + [d[0] for d in diff_non_trivial['deleted']]
    file_lizard_src = lizard.analyze_file.analyze_source_code(changed_file, source_code)
    mod_funcs = []
    for func in file_lizard_src.function_list:
        for line_num in mod_lines:
            if func.start_line <= line_num <= func.end_line:
                mod_funcs.append(func)
    mod_func = mod_funcs[0]

    # Get the first and last modified lines in the modified function
    # Restrict these lines to be in the modified function
    # Shift by delta_x and delta_y
    modified_lines = [ln for ln in modified_lines 
                      if ln >= mod_func.start_line and ln <= mod_func.end_line]

    x = max(min(modified_lines), mod_func.start_line)
    y = min(max(modified_lines), mod_func.end_line)

    modified_section = '\n'.join(source_code_lines[x-1:y])

    # Parse the source code with Tree-sitter
    parser = Parser(LANGUAGE)
    tree = parser.parse(bytes(source_code, 'utf8'))
    root_node = tree.root_node

    # Find the function containing the first modified line (TreeSitter)
    func_pattern = mod_func.long_name.split('::')[-1].replace('*', r'\*').replace('(', r'\s*\(').replace(')', r'\s*\)').replace('[', r'\s*\[').replace(']', r'\s*\]').replace('&', r'\s*&\s*').replace(',', r'\s*,\s*').replace(' ', r'\s*').replace('void', '') + r'\s*[\s\w:\(\),]*{'  # + r'\s*{'
    funcs = find_functions(root_node, func_pattern, x, y)
    if len(funcs) == 0:
        print(f"ID {id}: Tree sitter failed to find function node, using whole function as code block")
        func_text = '\n'.join(source_code_lines[mod_func.start_line-1:mod_func.end_line])
    elif len(funcs) > 1:
        funcs = find_closest_func(mod_func, funcs)
        if funcs is not None:
            function_node = funcs[0]
            func_text = function_node.text.decode('utf-8')
        else:
            print(f"ID {id}: Tree sitter failed to find function node, using whole function as code block")
            func_text = '\n'.join(source_code_lines[mod_func.start_line-1:mod_func.end_line])
    else: 
        function_node = funcs[0]
        func_text = function_node.text.decode('utf-8')
    
    # get spacing
    leading_spaces = get_leading_whitespace(sec_code_block)
    ending_spaces = get_leading_whitespace(sec_code_block[::-1])[::-1]

    # write mask_func (without description)
    mask_func = func_text.replace(sec_code_block, f"{leading_spaces}// <MASK>{ending_spaces}")
    with open(mask_func_file, 'w') as f:
        f.write(mask_func)
    
    # write mask_func_desc (with description)
    with open(desc_file, 'r') as f:
        desc = f.read()
    desc_lines = re.split(r'\n', desc)

    if leading_spaces == '':
        before_mask = func_text.split(sec_code_block)[0]
        before_mask_lines = re.split(r'\n', before_mask)
        leading_spaces = get_leading_whitespace(before_mask_lines[-1])
        if len(before_mask_lines[-1]) > len(leading_spaces):  # must be a non-space character in the line and before mask
            desc_lines[0] = '\n' + leading_spaces + desc_lines[0]
        desc_lines[1:] = [leading_spaces + ln for ln in desc_lines[1:]]
        mask_line = f"{leading_spaces}// <MASK>"
    else:
        desc_lines[0] = leading_spaces + desc_lines[0]
        leading_spaces_no_nl = leading_spaces.replace('\n', '')
        desc_lines[1:] = [leading_spaces_no_nl + ln for ln in desc_lines[1:]]
        mask_line = f"{leading_spaces_no_nl}// <MASK>{ending_spaces}"
    
    desc_mask_lines = desc_lines + [mask_line]
    desc_mask = '\n'.join(desc_mask_lines)
    mask_func_desc = func_text.replace(sec_code_block, desc_mask)

    with open(mask_func_desc_file, 'w') as f:
        f.write(mask_func_desc)


if __name__ == "__main__":
    with open('final_ids.txt', 'r') as f:
        ids_good = f.readlines()
    ids = [id.strip() for id in ids_good[1:]]

    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    base_path = '/home/cdilgren/project_benchmark/descriptions'
    if not os.path.exists(base_path):
        os.mkdir(base_path)

    for id in ids:
        print(id)
        diff_non_trivial = cases[id]['diff']
        changed_file = cases[id]['changed_file']
        mask_func(id, diff_non_trivial, changed_file, base_path)
