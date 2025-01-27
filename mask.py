import os
import re
import json
import random
import tree_sitter_c as tsc
import tree_sitter_cpp as tscpp
from tree_sitter import Language, Parser
import lizard
from utils import *
from get_new_var import APIEvaler


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


def get_new_var(function_node, old_var, evaler):
    # make prompt for LM to make a new_var
    prompt = f"Below is a C/C++ function. Create a new name for the variable {old_var}. "
    prompt += "The new variable name should be semantically similar to the original name and variable purpose. "
    prompt += "For example, a variable called 'dst' that tracks geometric distance can be renamed 'distance'. "
    prompt += "As another example, a variable called 'start_line can be renamed 's_ln'. "
    prompt += "Only return the new variable name. "
    prompt += "DO NOT include any other information, such as a preamble or suffix."
    prompt += "\n```\n"
    prompt += function_node.text.decode('utf-8')
    prompt += "\n```\n"

    new_var = evaler.get_response(prompt)
    return new_var


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


def mask_helper(id, case, base_path, mod, perturbed_content=None):
    delta_x = 0
    delta_y = 0
    sec_code_block, vul_code_block, x = mask(id, case, base_path, delta_x, delta_y, mod, perturbed_content)
    sec_code_block_comments_rm = remove_sl_comments_code_block(sec_code_block)
    vul_code_block_comments_rm = remove_sl_comments_code_block(vul_code_block)

    while sec_code_block_comments_rm.strip() == '' or vul_code_block_comments_rm.strip() == '':
        print(f'ID {id} has empty code block, adding surrounding line')
        if delta_x == delta_y:
            delta_y += 1
        else:
            delta_x += 1
        
        output = mask(id, case, base_path, delta_x, delta_y, mod, perturbed_content)
        if output is None:  # range exceeds modified function
            break
        else:
            sec_code_block, vul_code_block, x = output
            sec_code_block_comments_rm = remove_sl_comments_code_block(sec_code_block)
            vul_code_block_comments_rm = remove_sl_comments_code_block(vul_code_block)
    
    return x


def mask(id, case, base_path, delta_x, delta_y, mod, perturbed_content):

    print(f"Processing {id}, {mod}")

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
        mask_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_{mod}.c'
        sec_code_block_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_{mod}.c'
        vul_code_block_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/vul_code_block_{mod}.c'
        sec_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_{mod}.c'
        vul_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/vul_{mod}.c'
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
        mask_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_{mod}.cpp'
        sec_code_block_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_{mod}.cpp'
        vul_code_block_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/vul_code_block_{mod}.cpp'
        sec_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_{mod}.cpp'
        vul_file = f'/home/cdilgren/project_benchmark/descriptions/{id}/vul_{mod}.cpp'
    else:
        print(f"Language of modified file not recognized for id {id}")
        return

    # get sec source code, vul source code
    if mod == 'base':
        source_code = case['source_code']
        vul_source_code = case['source_code_before']
    elif mod == 'perturbed':
        source_code, vul_source_code = perturbed_content
    else:
        print("mod not recognized")
        return

    # write sec to file
    with open(sec_file, 'w') as f:
        f.write(source_code)
    print(f"Sec {mod} version written to {sec_file}")

    # write vul to file
    with open(vul_file, 'w') as f:
        f.write(vul_source_code)
    print(f"Vul {mod} version written to {vul_file}")

    # Read the modified source code -- use regex for \n to ignore special characters like FF \x0c
    source_code_lines = re.split(r'\n', source_code)
    total_lines = len(source_code_lines)

    # Get full diff file
    diff_file = f'/home/cdilgren/project_benchmark/ARVO-Meta/patches/{id}.diff'
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
    print(f"Code block {mod} replaced with // <MASK> in {mask_file}")

    # Write the sec code block to file
    with open(sec_code_block_file, 'w') as f:
        f.write(sec_code_block)
    print(f"Sec code block {mod} written to {sec_code_block_file}")

    # Get vul code block
    vul_code_block = get_vul_code_block(modified_source_code, sec_code_block, vul_source_code, diff)

    # Write the vul code block to file
    with open(vul_code_block_file, 'w') as f:
        f.write(vul_code_block)
    print(f"Vul code block {mod} written to {vul_code_block_file}")

    return sec_code_block, vul_code_block, x


def local_var_perturbation(id2var, evaler, case, x):
    # writes sec and vul files with one variable name change in target function

    # Determine the language based on file extension
    ext = get_file_extension(case['changed_file'])
    language = determine_language(ext)
    if language == 'c':
        LANGUAGE = C_LANGUAGE
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
    else:
        print(f"Language of modified file not recognized for id {id}")
        return

    # Get full diff file
    diff_file = f'/home/cdilgren/project_benchmark/ARVO-Meta/patches/{id}.diff'
    with open(diff_file, 'r') as f:
        diff_file_content = f.read()
    diff = parse_git_diff(diff_file_content, case['changed_file'])

    # get function node in sec and vul files
    parser = Parser(LANGUAGE)
    
    sec_tree = parser.parse(bytes(case['source_code'], 'utf8'))
    sec_function_node = get_ts_function_node(sec_tree.root_node, 'sec', diff, case['diff'], case['changed_file'], case['changed_function'], case['source_code'])

    vul_tree = parser.parse(bytes(case['source_code_before'], 'utf8'))
    vul_function_node = get_ts_function_node(vul_tree.root_node, 'vul', diff, case['diff'], case['changed_file'], case['changed_function'], case['source_code_before'])

    if sec_function_node is None or vul_function_node is None:
        # if tree-sitter can't find the function node, then we can't perturb
        id2var[id] = {
            'old_var': None,
            'new_var': None
        }
        print("No variables found in the function.")
        sec_perturbed_content = case['source_code']
        vul_perturbed_content = case['source_code_before']
    else:
        variables = find_variables(sec_function_node, x)

        if variables:
            variables_text = [var.text.decode('utf-8') for var in variables]

            if id in id2var.keys():
                old_var = id2var[id]['old_var']
                new_var = id2var[id]['new_var']
                if old_var is not None and new_var is not None and old_var in variables_text:
                    # use previous var
                    print(f"Using previous variable found: {old_var}, new var: {new_var}")
                else:
                    # mask changed, need to pick a new one
                    old_var = random.choice(variables_text)
                    new_var = get_new_var(sec_function_node, old_var, evaler)
                    id2var[id] = {
                        'old_var': old_var,
                        'new_var': new_var
                    }
                    print(f"Variable found: {old_var}, new var: {new_var}")
            else:
                # get new var
                old_var = random.choice(variables_text)
                new_var = get_new_var(sec_function_node, old_var, evaler)
                id2var[id] = {
                    'old_var': old_var,
                    'new_var': new_var
                }
                print(f"Variable found: {old_var}, new var: {new_var}")

            # replace var in sec, vul file
            sec_perturbed_content = replace_var_name(sec_function_node, old_var, new_var, case['source_code'])
            vul_perturbed_content = replace_var_name(vul_function_node, old_var, new_var, case['source_code_before'])

        else:
            # if there are no local vars before the mask, then we can't perturb
            id2var[id] = {
                'old_var': None,
                'new_var': None
            }
            print("No variables found in the function.")
            sec_perturbed_content = case['source_code']
            vul_perturbed_content = case['source_code_before']

    # Write id2var
    with open('id2var.json', 'w') as f:
        json.dump(id2var, f, indent=4)

    return sec_perturbed_content, vul_perturbed_content


if __name__ == "__main__":
    with open('ids_top40.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    base_path = '/home/cdilgren/project_benchmark/descriptions'
    if not os.path.exists(base_path):
        os.mkdir(base_path)

    with open('id2var.json', 'r') as f:
        id2var = json.load(f)

    evaler = APIEvaler()

    for id in ids:
        print(id)
        case = cases[id]
        x = mask_helper(id, case, base_path, 'base')
        perturbed_content = local_var_perturbation(id2var, evaler, case, x)
        mask_helper(id, case, base_path, 'perturbed', perturbed_content)
