import os
import glob
import re
import json
import random
import tree_sitter_c as tsc
import tree_sitter_cpp as tscpp
from tree_sitter import Language, Parser
import lizard
from utils import *
from insert_print import insert_print


def make_mangled_name(name, full_parameters):
    param_types = []
    for param in full_parameters:
        param = param.strip()
        if len(param.split(' ')) == 1:
            param_types.append(param)
        else:
            param_types.append(' '.join(param.split(' ')[:-1]))
    mangled_name = [name] + param_types
    return mangled_name


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
        if char.isspace():
            leading += char
        else:
            break
    return leading


def replace_code_block_with_mask(source_code, code_block):
    # Replaces the code in the given range with "// <MASK>"
    if isinstance(code_block, list):
        escaped_strings = [re.escape(block.text.decode('utf-8')) for block in code_block]
        pattern = r'[\s\S]*'.join(escaped_strings) if len(escaped_strings) > 1 else escaped_strings[0]
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
        pattern = re.escape(code_block_text)

    # Replace the block with "// <MASK>", near start_byte and end_byte
    search_section = source_code[start_byte:end_byte]
    match = re.search(pattern, search_section)
    while not match:
        if start_byte == 0 and end_byte == len(source_code) - 1:
            return
        if start_byte > 0:
            start_byte += -1
        if end_byte < len(source_code) - 1:
            end_byte += 1
        search_section = source_code[start_byte:end_byte]
        match = re.search(pattern, search_section)

    code_block_text = match.group()

    masked_section = search_section[:match.span()[0]] + "// <MASK>" + search_section[match.span()[1]:]
    masked_code = source_code[:start_byte] + masked_section + source_code[end_byte:]
    return masked_code, code_block_text


def get_vul_code_block(modified_source_code, sec_code_block, vul_source_code, diff):
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
            elif ln_num == start_row + 1:
                start_col = 0
            if ln_num <= end_row + 2:  # possible to insert del line after sec code block, see sample 13180
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
            elif ln_num == start_row + 1:
                start_col = 0
            if ln_num <= end_row + 2:  # possible to insert del line after sec code block, see sample 13180
                end_row += 1
            added_lines = [ln+1 for ln in added_lines]

    # Read the vul source code -- use regex for \n to ignore special characters like FF \x0c
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


def remove_sl_comments_code_block(code_block):
    code_block_lines = re.split(r'\n', code_block)
    code_block_lines = [remove_comments(line) for line in code_block_lines]
    return '\n'.join(code_block_lines)


def get_nontrivial_line_len(code_block):
    # count lines that are not comments or blank
    code_block_comments_rm = remove_sl_comments_code_block(code_block)
    code_block_lines = re.split(r'\n', code_block_comments_rm)
    code_block_lines = [ln for ln in code_block_lines if ln.strip() != '']

    return len(code_block_lines)


def find_functions(node, func_pattern, x, y):
    """Recursively finds function definitions and their variable declarations."""
    results = []

    def traverse(node, x, y):
        if node.type == 'function_definition':
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
    # Find the natural code block
    sec_code_block = find_code_block(function_node, x, y, total_lines, modified_section, consider_sibling=True)

    # Replace the code block with a non-op comment "// <MASK>"
    if sec_code_block is None:
        # tree sitter failed, use whole function as code block
        print(f"ID {id}: Tree sitter failed to find code block, using whole function as code block")
        modified_source_code, sec_code_block = get_function_content(mod_func, source_code_lines)
    else:
        # Replace the identified block with the comment
        modified_source_code, sec_code_block = replace_code_block_with_mask(source_code, sec_code_block)

    # fix spacing - shift leading and trailing spaces in code block to mask
    leading_spaces = get_leading_whitespace(sec_code_block)
    ending_spaces = get_leading_whitespace(sec_code_block[::-1])[::-1]
    modified_source_code = modified_source_code.replace("// <MASK>", f"{leading_spaces}// <MASK>{ending_spaces}")
    sec_code_block = sec_code_block.strip()

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


def get_mod_lines_added(diff, diff_non_trivial):
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
    
    return modified_lines


def get_mod_lines_deleted(diff, diff_non_trivial):
    # Get the modified line numbers in the source file for added and deleted
    modified_lines = [change[0] for change in diff_non_trivial['deleted']]

    added_lines = [change[0] for change in diff['added']]
    deleted_lines = [change[0] for change in diff['deleted']]
    added_nt_lines = [change[0] for change in diff_non_trivial['added']]

    while len(added_lines) != 0 or len(deleted_lines) != 0:
        if len(deleted_lines) == 0:
            line = added_lines.pop(0)
            if line not in modified_lines and line in added_nt_lines:
                added_nt_lines.remove(line)
                modified_lines.append(line)
            added_lines = [line - 1 for line in added_lines]
            added_nt_lines = [line - 1 for line in added_nt_lines]
        elif len(added_lines) == 0:
            break
        elif added_lines[0] == deleted_lines[0]:
            line = added_lines.pop(0)
            deleted_lines.pop(0)
            if line not in modified_lines and line in added_nt_lines:
                added_nt_lines.remove(line)
                modified_lines.append(line)
        elif added_lines[0] < deleted_lines[0]:
            line = added_lines.pop(0)
            if line not in modified_lines and line in added_nt_lines:
                added_nt_lines.remove(line)
                modified_lines.append(line)
            added_lines = [line - 1 for line in added_lines]
            added_nt_lines = [line - 1 for line in added_nt_lines]
        else:
            added_lines = [line + 1 for line in added_lines]
            added_nt_lines = [line + 1 for line in added_nt_lines]
            deleted_lines.pop(0)

    modified_lines = sorted(modified_lines)

    return modified_lines


def get_ts_function_node(root_node, version, diff, diff_non_trivial, changed_file, changed_function, source_code):

    # Find the function containing the first modified line (lizard)
    if version == 'sec':
        modified_lines = get_mod_lines_added(diff, diff_non_trivial)
    else:
        modified_lines = get_mod_lines_deleted(diff, diff_non_trivial)
    file_lizard_src = lizard.analyze_file.analyze_source_code(changed_file, source_code)
    mod_funcs = []
    for line_num in modified_lines:
        for func in file_lizard_src.function_list:
            if func.start_line <= line_num <= func.end_line and changed_function == make_mangled_name(func.name, func.full_parameters) and func not in mod_funcs:
                mod_funcs.append(func)
    mod_func = mod_funcs[0]

    # get line numbers in the function, doesn't need to be precise range of modified lines
    x = int((mod_func.start_line + mod_func.end_line) / 2)
    y = x + 1

    # Find the function containing the first modified line (TreeSitter)
    func_pattern = mod_func.long_name.split('::')[-1].replace('*', r'\*').replace('(', r'\s*\(').replace(')', r'\s*\)').replace('[', r'\s*\[').replace(']', r'\s*\]').replace('&', r'\s*&\s*').replace(',', r'\s*,\s*').replace(' ', r'\s*').replace('void', '') + r'\s*[\s\w:\(\),]*{'  # + r'\s*{'
    funcs = find_functions(root_node, func_pattern, x, y)
    function_node = None
    if len(funcs) > 1:
        funcs = find_closest_func(mod_func, funcs)
        if funcs is not None:
            function_node = funcs[0]
    elif len(funcs) == 1: 
        function_node = funcs[0]

    return function_node


def get_func_text(function_node, mod_func, source_code_lines):
    if function_node is None:
        func_text = '\n'.join(source_code_lines[mod_func.start_line-1:mod_func.end_line])
    else:
        func_text = function_node.text.decode('utf-8')
    return func_text


def get_mask_func_text(func_text, sec_code_block):
    # get spacing
    leading_spaces = get_leading_whitespace(sec_code_block)
    ending_spaces = get_leading_whitespace(sec_code_block[::-1])[::-1]

    # write mask_func (without description)
    mask_func = func_text.replace(sec_code_block, f"{leading_spaces}// <MASK>{ending_spaces}")

    return mask_func


def make_vul_sec_base_file(mask_content, vul_code_block):
    # create mod file (sec file base with the LM patch)
    mod_file_content = mask_content.replace("// <MASK>", vul_code_block)
    return mod_file_content


def mask_helper(id, case, base_path):
    delta_x = 0
    delta_y = 0
    sec_code_block, vul_code_block = mask(id, case, base_path, delta_x, delta_y)

    sec_line_len = get_nontrivial_line_len(sec_code_block)
    vul_line_len = get_nontrivial_line_len(vul_code_block)

    while sec_line_len < 10 or vul_line_len < 10:
        print(f'ID {id} has code block <10 lines, adding surrounding line')
        if delta_x == delta_y:
            delta_y += 1
        else:
            delta_x += 1
        
        output = mask(id, case, base_path, delta_x, delta_y)
        if output is None:  # range exceeds modified function
            break
        else:
            sec_code_block, vul_code_block = output
            sec_line_len = get_nontrivial_line_len(sec_code_block)
            vul_line_len = get_nontrivial_line_len(vul_code_block)


def mask(id, case, base_path, delta_x, delta_y):

    base_path_id = os.path.join(base_path, id)
    if not os.path.exists(base_path_id):
        os.mkdir(base_path_id)

    # unpack case
    changed_file = case['changed_file']
    diff_non_trivial = case['diff']

    # Determine the language based on file extension
    ext = get_file_extension(changed_file)
    language = determine_language(ext)
    if language == 'c':
        LANGUAGE = C_LANGUAGE
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
    else:
        print(f"Language of modified file not recognized for id {id}")
        return
    
    # output file names
    sec_code_block_file = f'descriptions/{id}/sec_code_block_base.{language}'
    vul_code_block_file = f'descriptions/{id}/vul_code_block_base.{language}'
    sec_file = f'descriptions/{id}/sec_base.{language}'
    vul_file = f'descriptions/{id}/vul_base.{language}'
    mask_file = f'descriptions/{id}/mask_base.{language}'
    sec_print_file = f'descriptions/{id}/sec_print_base.{language}'
    sec_func_file = f'descriptions/{id}/sec_func_base.{language}'
    mask_sec_func_file = f'descriptions/{id}/mask_sec_func_base.{language}'
    vul_sec_base_file = f'descriptions/{id}/vul_sec_base_base.{language}'

    # get sec source code, vul source code
    source_code = case['source_code']
    vul_source_code = case['source_code_before']

    # write sec to file
    with open(sec_file, 'w') as f:
        f.write(source_code)
    # print(f"Sec file written to {sec_file}")

    # write vul to file
    with open(vul_file, 'w') as f:
        f.write(vul_source_code)
    # print(f"Vul file written to {vul_file}")

    # Read the modified source code -- use regex for \n to ignore special characters like FF \x0c
    source_code_lines = re.split(r'\n', source_code)
    total_lines = len(source_code_lines)

    # Get full diff file
    diff_file = f'ARVO-Meta/patches/{id}.diff'
    with open(diff_file, 'r') as f:
        diff_file_content = f.read()
    diff = parse_git_diff(diff_file_content, changed_file)

    # Get the modified line numbers in the source file for added and deleted
    modified_lines = get_mod_lines_added(diff, diff_non_trivial)

    # Find the function containing the first modified line (lizard)
    file_lizard_src = lizard.analyze_file.analyze_source_code(changed_file, source_code)
    file_lizard_src_base = lizard.analyze_file.analyze_source_code(changed_file, case['source_code'])
    mod_funcs_i = []
    for line_num in modified_lines:
        for i, func in enumerate(file_lizard_src_base.function_list):
            if func.start_line <= line_num <= func.end_line and case['changed_function'] == make_mangled_name(func.name, func.full_parameters) and i not in mod_funcs_i:
                mod_funcs_i.append(i)
    mod_func = file_lizard_src.function_list[mod_funcs_i[0]]

    # Get the first and last modified lines in the modified function
    # Restrict these lines to be in the modified function
    # Shift by delta_x and delta_y
    modified_lines = [ln for ln in modified_lines 
                      if ln >= mod_func.start_line and ln <= mod_func.end_line]

    if (min(modified_lines) - delta_x < mod_func.start_line) and (max(modified_lines) + delta_y > mod_func.end_line):
        print(f'ID {id}: Entire function added, so no vul code block')
        return

    x = max(min(modified_lines) - delta_x, mod_func.start_line)
    y = min(max(modified_lines) + delta_y, mod_func.end_line)

    modified_section = '\n'.join(source_code_lines[x-1:y])

    # Parse the source code with Tree-sitter
    parser = Parser(LANGUAGE)
    tree = parser.parse(bytes(source_code, 'utf8'))
    root_node = tree.root_node

    # Find the function containing the first modified line (TreeSitter)
    func_pattern = mod_func.long_name.split('::')[-1].replace('*', r'\*').replace('(', r'\s*\(').replace(')', r'\s*\)').replace('[', r'\s*\[').replace(']', r'\s*\]').replace('&', r'\s*&\s*').replace(',', r'\s*,\s*').replace(' ', r'\s*').replace('void', '') + r'\s*[\s\w:\(\),]*{'  # + r'\s*{'
    funcs = find_functions(root_node, func_pattern, x, y)
    function_node = None
    if len(funcs) == 0:
        print(f"ID {id}: Tree sitter failed to find function node, using whole function as code block")
        mask_source_code, sec_code_block = get_function_content(mod_func, source_code_lines)
    elif len(funcs) > 1:
        funcs = find_closest_func(mod_func, funcs)
        if funcs is not None:
            function_node = funcs[0]
            mask_source_code, sec_code_block = get_code_block(function_node, x, y, total_lines, source_code, mod_func, source_code_lines, modified_section)
        else:
            print(f"ID {id}: Tree sitter failed to find function node, using whole function as code block")
            mask_source_code, sec_code_block = get_function_content(mod_func, source_code_lines)
    else: 
        function_node = funcs[0]
        mask_source_code, sec_code_block = get_code_block(function_node, x, y, total_lines, source_code, mod_func, source_code_lines, modified_section)

    # Write the modified source code back to the file (or write to a new file)
    with open(mask_file, 'w') as f:
        f.write(mask_source_code)
    # print(f"Masked sec file written to {mask_file}")

    # Write the sec code block to file
    with open(sec_code_block_file, 'w') as f:
        f.write(sec_code_block)
    # print(f"Sec code block written to {sec_code_block_file}")

    # Get vul code block
    vul_code_block = get_vul_code_block(mask_source_code, sec_code_block, vul_source_code, diff)

    # Write the vul code block to file
    with open(vul_code_block_file, 'w') as f:
        f.write(vul_code_block)
    # print(f"Vul code block written to {vul_code_block_file}")

    # write the sec function to file
    func_text = get_func_text(function_node, mod_func, source_code_lines)
    with open(sec_func_file, 'w') as f:
        f.write(func_text)
    # print(f"Sec func written to {sec_func_file}")

    # write sec version with the inserted print to file
    sec_print = insert_print(function_node, mod_func, source_code_lines)
    with open(sec_print_file, 'w') as f:
        f.write(sec_print)
    # print(f"Sec print written to {sec_print_file}")

    # write the masked sec function to file
    mask_func_text = get_mask_func_text(func_text, sec_code_block)
    with open(mask_sec_func_file, 'w') as f:
        f.write(mask_func_text)
    # print(f"Masked sec func written to {mask_sec_func_file}")

    # write the vul with sec base to file
    vul_sec_base = make_vul_sec_base_file(mask_source_code, vul_code_block)
    with open(vul_sec_base_file, 'w') as f:
        f.write(vul_sec_base)
    # print(f"Vul sec base written to {vul_sec_base_file}")

    return sec_code_block, vul_code_block


if __name__ == "__main__":
    with open('ids_new.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    base_path = 'descriptions'
    if not os.path.exists(base_path):
        os.mkdir(base_path)

    for id in ids:
        # remove current files to keep this clean - we're basically restarting
        directory = f'descriptions/{id}'
        for file in glob.glob(os.path.join(directory, "*")):
            os.remove(file)

        # print(id)
        case = cases[id]
        mask_helper(id, case, base_path)
