import sys
import os
import re
import csv
import json
import difflib
from tree_sitter import Language, Parser
import lizard
from utils import *


def replace_code_block_with_mask(source_code, code_block):
    # Replaces the code in the given range with "// <MASK>"
    if isinstance(code_block, list):
        code_block_text = ''
        for block in code_block:
            code_block_text += block.text.decode('utf-8')
        start_byte = code_block[0].start_byte
        end_byte = code_block[-1].end_byte
    else:
        code_block_text = code_block.text.decode('utf-8')
        # Check if the code_block is a 'compound_statement' and adjust accordingly
        if code_block.type == 'compound_statement':
            # Exclude the braces
            if code_block_text.startswith('{') and code_block_text.endswith('}'):
                code_block_text = code_block_text[1:-1]
                start_byte = code_block.children[0].end_byte  # End of '{'
                end_byte = code_block.children[-1].start_byte  # Start of '}'
                # Handle empty function bodies
                if code_block_text == '':
                    # Empty body, nothing to mask
                    return
            else:
                # No braces found, treat as empty
                return
        else:
            start_byte = code_block.start_byte
            end_byte = code_block.end_byte

    # Replace the block with "// <MASK>", near start_byte and end_byte
    search_section = source_code[start_byte:end_byte]
    while code_block_text not in search_section:
        if start_byte == 0 and end_byte == len(source_code) - 1:
            return
        if start_byte > 0:
            start_byte += -1
        if end_byte < len(source_code) - 1:
            end_byte += 1
        search_section = source_code[start_byte:end_byte]

    code_block_text_start = search_section.find(code_block_text)
    code_block_text_end = code_block_text_start + len(code_block_text)

    masked_section = search_section[:code_block_text_start] + "// <MASK>" + search_section[code_block_text_end:]
    masked_code = source_code[:start_byte] + masked_section + source_code[end_byte:]
    return masked_code, code_block_text


def get_vul_code_block(modified_source_code, sec_code_block, vul_source_file, diff):
    # get sec_code_block start (row, col)
    before_mask = modified_source_code.split('// <MASK>')[0]
    before_mask_lines = re.split(r'\n', before_mask)
    start_row = len(before_mask_lines) - 1
    start_col = len(before_mask_lines[-1])

    # get sec_code_block end (row, col)
    up_to_block = before_mask + sec_code_block
    up_to_block_lines = re.split(r'\n', up_to_block)
    end_row = len(up_to_block_lines) - 1
    end_col = len(up_to_block_lines[-1]) - 1

    # get content after mask and on same line as end
    after_mask = modified_source_code.split('// <MASK>')[1]
    sec_source_code = up_to_block + after_mask
    sec_source_code_lines = re.split(r'\n', sec_source_code)
    post_mask_line = sec_source_code_lines[end_row][end_col+1:]

    # translate sec_code_block start and end to vul_code_block file using diff
    added_lines = [change[0] for change in diff['added']]
    deleted_lines = [change[0] for change in diff['deleted']]

    while len(added_lines) != 0 or len(deleted_lines) != 0:
        if len(added_lines) == 0:
            ln_num = deleted_lines.pop(0)
            if ln_num < start_row + 1:
                start_row += 1
            if ln_num < end_row + 1:
                end_row += 1
            elif ln_num == end_row + 1:
                end_row += 1

        elif len(deleted_lines) == 0:
            ln_num = added_lines.pop(0)
            if ln_num < start_row + 1:
                start_row -= 1
            if ln_num < end_row + 1:
                end_row -= 1
            elif ln_num == end_row + 1:
                end_row -= 1
            added_lines = [ln-1 for ln in added_lines]

        elif added_lines[0] == deleted_lines[0]:
            deleted_lines.pop(0)
            added_lines.pop(0)

        elif added_lines[0] < deleted_lines[0]:
            ln_num = added_lines.pop(0)
            if ln_num < start_row + 1:
                start_row -= 1
            if ln_num < end_row + 1:
                end_row -= 1
            elif ln_num == end_row + 1:
                end_row -= 1
            added_lines = [ln-1 for ln in added_lines]

        else:
            ln_num = deleted_lines.pop(0)
            if ln_num < start_row + 1:
                start_row += 1
            if ln_num < end_row + 1:
                end_row += 1
            elif ln_num == end_row + 1:
                end_row += 1
            added_lines = [ln+1 for ln in added_lines]

    # Read the vul source code -- use regex for \n to ignore special characters like FF \x0c
    with open(vul_source_file, 'r') as f:
        vul_source_code = f.read()
    vul_source_code_lines = re.split(r'\n', vul_source_code)

    # get vul code block
    vul_code_block_lines = vul_source_code_lines[start_row:end_row+1]

    # shift start and end lines
    if len(vul_code_block_lines) > 0:
        end_column = vul_code_block_lines[-1].rfind(post_mask_line)
        vul_code_block_lines[-1] = vul_code_block_lines[-1][:end_column]
        vul_code_block_lines[0] = vul_code_block_lines[0][start_col:]

    vul_code_block = '\n'.join(vul_code_block_lines)

    return vul_code_block


def remove_comments(input_string):
    # Pattern to match single-line comments (//) and multi-line comments (/* */)
    comment_pattern = re.compile(r'(//.*?$|/\*.*?\*/)', re.DOTALL | re.MULTILINE)
    
    # Replace comments with an empty string
    result = re.sub(comment_pattern, '', input_string)
    
    return result


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


def get_code_block(function_node, x, y, total_lines, source_code, mod_func, source_code_lines, modified_section):
    # for single line changes, increase scope +3 lines each side
    if x == y:
        x = max(x-3, function_node.start_point[0] + 1)
        y = min(y+3, function_node.end_point[0] + 1)
    
    # Find the natural code block
    sec_code_block = find_code_block(function_node, x, y, total_lines, modified_section)

    # Replace the code block with a non-op comment "// <MASK>"
    if sec_code_block is None:
        # tree sitter failed, use whole function as code block
        print(f"ID {id}: Tree sitter failed to find code block, using whole function as code block")
        modified_source_code, sec_code_block = get_function_content(mod_func, source_code_lines)
        return modified_source_code, sec_code_block
    else:
        # Replace the identified block with the comment
        modified_source_code, sec_code_block = replace_code_block_with_mask(source_code, sec_code_block)

    return modified_source_code, sec_code_block


def get_function_content(mod_func, source_code_lines):
    # start_line is line after opening bracket
    start_line = mod_func.start_line - 1
    while '{' not in source_code_lines[start_line]:
        start_line += 1
    start_line += 1

    # end line is line before closing bracket
    end_line = mod_func.end_line - 1
    while '}' not in source_code_lines[end_line]:
        end_line += 1

    sec_code_block = '\n'.join(source_code_lines[start_line:end_line])
    
    masked_code = '\n'.join(source_code_lines[:start_line]) 
    masked_code += "\n// <MASK>\n"
    masked_code += '\n'.join(source_code_lines[end_line:])

    return masked_code, sec_code_block


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


def mask_helper(id, diff_non_trivial, changed_file, base_path):
    delta_x = 0
    delta_y = 0
    sec_code_block, vul_code_block = mask(id, diff_non_trivial, changed_file, base_path, delta_x, delta_y)

    while sec_code_block.strip() == '' or vul_code_block.strip() == '':
        print(f'ID {id} has empty code block, adding surrounding line')
        if delta_x == delta_y:
            delta_y += 1
        else:
            delta_x += 1
        
        output = mask(id, diff_non_trivial, changed_file, base_path, delta_x, delta_y)
        if output is None:  # range exceeds modified function
            break
        else:
            sec_code_block, vul_code_block = output


def mask(id, diff_non_trivial, changed_file, base_path, delta_x, delta_y):

    print(f"Processing {id}")

    # set paths
    sec_source_file = f'/data/cmd-oss-fuzz-bench/{id}/patches/sec.txt'
    vul_source_file = f'/data/cmd-oss-fuzz-bench/{id}/patches/vul.txt'
    mask_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/mask.txt'
    sec_code_block_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/sec_code_block.txt'
    vul_code_block_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/vul_code_block.txt'
    diff_file = f'/space1/cdilgren/project_benchmark/ARVO-Meta/patches/{id}.diff'

    base_path_id = os.path.join(base_path, id)
    if not os.path.exists(base_path_id):
        os.mkdir(base_path_id)

    # Determine the language based on file extension
    ext = get_file_extension(changed_file)
    language = determine_language(ext)
    if language == 'c':
        LANGUAGE = C_LANGUAGE
        mask_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/mask.c'
        sec_code_block_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/sec_code_block.c'
        vul_code_block_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/vul_code_block.c'
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
        mask_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/mask.cpp'
        sec_code_block_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/sec_code_block.cpp'
        vul_code_block_file = f'/space1/cdilgren/project_benchmark/descriptions/{id}/vul_code_block.cpp'
    else:
        print(f"Language of modified file not recognized for id {id}")
        return

    # Read the modified source code -- use regex for \n to ignore special characters like FF \x0c
    with open(sec_source_file, 'r') as f:
        source_code = f.read()
    source_code_lines = re.split(r'\n', source_code)
    total_lines = len(source_code_lines)

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
                      if ln >= mod_func.start_line-1 and ln <= mod_func.end_line-1]

    if (min(modified_lines) - delta_x < mod_func.start_line - 1) and (max(modified_lines) + delta_y > mod_func.end_line - 1):
        print(f'ID {id}: Entire function added, so no vul code block')
        return

    x = max(min(modified_lines) - delta_x, mod_func.start_line - 1)
    y = min(max(modified_lines) + delta_y, mod_func.end_line - 1)

    modified_section = '\n'.join(source_code_lines[x-1:y])

    # Parse the source code with Tree-sitter
    parser = Parser()
    parser.set_language(LANGUAGE)
    tree = parser.parse(bytes(source_code, 'utf8'))
    root_node = tree.root_node

    # Find the function containing the first modified line (TreeSitter)
    func_pattern = mod_func.long_name.split('::')[-1].replace('*', r'\*').replace('(', r'\s*\(').replace(')', r'\s*\)').replace('[', r'\s*\[').replace(']', r'\s*\]').replace('&', r'\s*&\s*').replace(',', r'\s*,\s*').replace(' ', r'\s*').replace('void', '') + r'\s*[\s\w:\(\),]*{'  # + r'\s*{'
    funcs = find_functions(root_node, func_pattern, x, y)
    if len(funcs) == 0:
        print(f"ID {id}: Tree sitter failed to find function node, using whole function as code block")
        modified_source_code, sec_code_block = get_function_content(mod_func, source_code_lines)
    elif len(funcs) > 1:
        funcs = find_closest_func(mod_func, funcs)
        if funcs is not None:
            function_node = funcs[0]
            modified_source_code, sec_code_block = get_code_block(function_node, x, y, total_lines, source_code, mod_func, source_code_lines, modified_section)
        else:
            print(f"ID {id}: Tree sitter failed to find function node, using whole function as code block")
            modified_source_code, sec_code_block = get_function_content(mod_func, source_code_lines)
    else: 
        function_node = funcs[0]
        modified_source_code, sec_code_block = get_code_block(function_node, x, y, total_lines, source_code, mod_func, source_code_lines, modified_section)
    
    # Write the modified source code back to the file (or write to a new file)
    with open(mask_file, 'w') as f:
        f.write(modified_source_code)
    print(f"Code block replaced with // <MASK> in {mask_file}")

    # Write the sec code block to file
    with open(sec_code_block_file, 'w') as f:
        f.write(sec_code_block)
    print(f"Sec code block written to {sec_code_block_file}")

    # Get vul code block
    vul_code_block = get_vul_code_block(modified_source_code, sec_code_block, vul_source_file, diff)

    # Write the vul code block to file
    with open(vul_code_block_file, 'w') as f:
        f.write(vul_code_block)
    print(f"Vul code block written to {vul_code_block_file}")

    return sec_code_block, vul_code_block


if __name__ == "__main__":
    # with open('filter_logs_all/analyze_report_unittest_testcase/ids_each_step.json', 'r') as f:
    #     ids_each_step = json.load(f)
    # ids = ids_each_step['ids_pass_testcase_unittest']

    with open('ids_good.txt', 'r') as f:
        ids_good = f.readlines()
    ids = [id.strip() for id in ids_good[1:]]

    with open('filter_logs_all/cases.json', 'r') as f:
        cases = json.load(f)

    base_path = '/space1/cdilgren/project_benchmark/descriptions'
    if not os.path.exists(base_path):
        os.mkdir(base_path)

    for id in ids:
        print(id)
        diff_non_trivial = cases[id]['diff']
        changed_file = cases[id]['changed_file']
        mask_helper(id, diff_non_trivial, changed_file, base_path)
