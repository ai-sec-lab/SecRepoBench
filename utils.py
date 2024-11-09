import sys
import os
import re
from tree_sitter import Language

language_path = '/space1/yanjun/instruction_tuning/tree_sitter/my-languages.so'
C_LANGUAGE = Language(language_path, 'c')
CPP_LANGUAGE = Language(language_path, 'cpp')

def determine_language(file_extension):
    c_extensions = ['.c', '.h']
    cpp_extensions = ['.cpp', '.cc', '.cxx', '.hpp', '.hxx', '.h']

    if file_extension in c_extensions:
        return 'c'
    elif file_extension in cpp_extensions:
        return 'cpp'
    else:
        # Return None for non-C/C++ files
        return None

def parse_diff_file(diff_file):
    modified_lines = []
    target_file = None
    processing_target_file = False

    with open(diff_file, 'r') as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('diff --git'):
            # Process diff line
            parts = line.strip().split()
            if len(parts) >= 3:
                a_file = parts[2][2:] if parts[2].startswith('a/') else parts[2]
                b_file = parts[3][2:] if parts[3].startswith('b/') else parts[3]
                current_file = os.path.normpath(b_file)
                file_extension = os.path.splitext(current_file)[1]
                language = determine_language(file_extension)
                if language and 'test' not in current_file:
                    # Found the target C/C++ file, rule out unit test/fuzzing files
                    target_file = current_file
                    processing_target_file = True
                else:
                    processing_target_file = False
            i += 1  # Increment i here
        elif line.startswith('@@') and processing_target_file:
            # Process hunk
            match = re.match(r'@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', line)
            if match:
                orig_start = int(match.group(1))
                orig_count = int(match.group(2)) if match.group(2) else 1
                mod_start = int(match.group(3))
                mod_count = int(match.group(4)) if match.group(4) else 1

                i += 1  # Move to the next line after @@
                orig_line_num = orig_start
                mod_line_num = mod_start

                while i < len(lines) and not lines[i].startswith('@@') and not lines[i].startswith('diff --git'):
                    hunk_line = lines[i]
                    if hunk_line.startswith('-'):
                        # Line removed from original file
                        modified_lines.append(mod_line_num)
                        orig_line_num += 1
                    elif hunk_line.startswith('+'):
                        # Line added to modified file
                        modified_lines.append(mod_line_num)
                        mod_line_num += 1
                    else:
                        # Context line (unchanged)
                        orig_line_num += 1
                        mod_line_num += 1
                    i += 1  # Increment i within the hunk
                # Do not increment i here; it's already incremented in the while loop
            else:
                i += 1  # Increment i if the @@ line doesn't match the expected format
        else:
            i += 1  # Increment i in all other cases
    # Return the target file and modified lines
    if not target_file:
        print("No C/C++ file found in the diff file.")
        sys.exit(1)

    return target_file, modified_lines

def get_file_extension(file_name):
    _, ext = os.path.splitext(file_name)
    return ext

def find_function_containing_line(tree, line_number):
    root_node = tree.root_node
    function_nodes = []

    def traverse(node):
        if node.type == 'function_definition':
            start_line = node.start_point[0] + 1  # Convert to 1-based index
            end_line = node.end_point[0] + 1
            if start_line <= line_number <= end_line:
                function_nodes.append(node)
        else:
            for child in node.children:
                traverse(child)

    traverse(root_node)

    if function_nodes:
        return function_nodes[0]  # Function containing the line
    else:
        # If no function contains the line, try the function before
        # Find the function whose end_line is less than line_number
        previous_function = None
        def find_previous_function(node):
            nonlocal previous_function
            if node.type == 'function_definition':
                start_line = node.start_point[0] + 1
                end_line = node.end_point[0] + 1
                if end_line < line_number:
                    previous_function = node
                elif start_line > line_number:
                    return
            for child in node.children:
                find_previous_function(child)

        find_previous_function(root_node)
        return previous_function

def find_code_block(node, x, y, total_lines, consider_sibling=False):
    y = min(y, total_lines)

    # Start from the function body (compound_statement)
    function_body_node = None
    for child in node.children:
        if child.type == 'compound_statement':
            function_body_node = child
            break

    if function_body_node is None:
        return None
    
    body_start_line = function_body_node.start_point[0] + 1
    body_end_line = function_body_node.end_point[0] + 1

    # Check if body_start_line or body_end_line corresponds to a bracket and adjust
    if function_body_node.children:
        if function_body_node.children[0].type == '{':
            body_start_line += 1  # Skip the opening bracket
        if function_body_node.children[-1].type == '}':
            body_end_line -= 1  # Skip the closing bracket

    # Ensure x does not go beyond the function's start line
    x = max(x, body_start_line)

    # Ensure y does not go beyond the function's end line
    y = min(y, body_end_line)

    # List of node types considered as single-line statements
    single_line_statement_types = [
        'expression_statement',
        'declaration',
        'return_statement',
        'break_statement',
        'continue_statement',
        'goto_statement',
        'labeled_statement'
    ]

    # Recursive function to find the smallest node covering the range
    def recursive_find(node):
        node_start_line = node.start_point[0] + 1
        node_end_line = node.end_point[0] + 1

        # Check if the node covers the range
        if node_start_line <= x and node_end_line >= y:
            # Try to find a smaller child node
            # if node.type in single_line_statement_types:
            #     return node
            for child in node.children:
                result = recursive_find(child)
                if result:
                    return result
            # If no smaller child covers the range, return this node
            return node
        else:
            return None  # This node doesn't cover the range

    # Find the smallest node covering the range within the function body
    minimal_node = recursive_find(function_body_node)

    if minimal_node is None:
        # If no node covers the range, default to function body
        minimal_node = function_body_node

    if consider_sibling:
        # Now, attempt to find a minimal combination of adjacent siblings
        # within the minimal_node
        if minimal_node.children:
            # Exclude braces if it's a compound_statement
            if minimal_node.type == 'compound_statement':
                statements = [child for child in minimal_node.children if child.type not in ('{', '}')]
            else:
                statements = minimal_node.children

            min_length = float('inf')
            selected_nodes = None

            for i in range(len(statements)):
                current_start_line = statements[i].start_point[0] + 1
                current_end_line = statements[i].end_point[0] + 1

                if current_end_line < x:
                    continue

                temp_nodes = []
                temp_start_line = current_start_line
                temp_end_line = current_end_line

                for j in range(i, len(statements)):
                    stmt = statements[j]
                    stmt_start_line = stmt.start_point[0] + 1
                    stmt_end_line = stmt.end_point[0] + 1

                    temp_nodes.append(stmt)
                    temp_end_line = stmt_end_line

                    if temp_start_line <= x and temp_end_line >= y:
                        length = temp_end_line - temp_start_line
                        if length < min_length:
                            min_length = length
                            selected_nodes = list(temp_nodes)
                        break  # Can't get a smaller combination starting at i

                    if temp_end_line > y:
                        break  # No need to include more statements

                if selected_nodes:
                    break  # Found the minimal combination

            if selected_nodes:
                return selected_nodes

    # If no smaller combination is found, return the minimal node
    return minimal_node

def get_function_name(node):
    def traverse(node):
        for ch in node.children:
            if ch.type == 'identifier':
                return ch.text.decode('utf8')
            else:
                return traverse(ch)
        return None

    for child in node.children:
        if child.type == 'function_declarator':
            for grandchild in child.children:
                if grandchild.type == 'identifier':
                    return grandchild.text.decode('utf8')
                if grandchild.type == 'qualified_identifier':
                    for great_grandchild in grandchild.children:
                        if great_grandchild.type == 'identifier':
                            return great_grandchild.text.decode('utf8')
        if child.type == 'pointer_declarator':
            for grandchild in child.children:
                if grandchild.type == 'function_declarator':
                    for great_grandchild in grandchild.children:
                        if great_grandchild.type == 'identifier':
                            return great_grandchild.text.decode('utf8')
    return None

def find_function_by_name(tree, function_name):
    root_node = tree.root_node

    def recursive_search(node):
        if node.type == 'function_definition':
            current_function_name = get_function_name(node)
            if current_function_name == function_name: #and '<<REGION START>>' in node.text.decode('utf-8'):
                return node
        
        for child in node.children:
            result = recursive_search(child)
            if result:
                return result
        
        return None

    result = recursive_search(root_node)
    return result

def find_mask_comment(node):
    if node.type == 'comment':
        comment_text = node.text.decode('utf-8').strip()
        if '<MASK>' in comment_text:
            return node
    for child in node.children:
        result = find_mask_comment(child)
        if result is not None:
            return result
    return None

def find_variables(node):
    mask_comment = find_mask_comment(node)
    variables = []

    def recursive_find(current_node):
        if current_node.start_byte >= mask_comment.start_byte:
            return

        if current_node.type == 'declaration':
            for child in current_node.children:
                if child.type in ['init_declarator', 'identifier']:
                    for grandchild in child.children:
                        if grandchild.type == 'identifier':
                            variables.append(grandchild)
                    if child.type == 'identifier':
                        variables.append(child)
        elif current_node.type == 'parameter_declaration':
            for child in current_node.children:
                if child.type == 'identifier':
                    variables.append(child)
                if child.type == 'pointer_declarator':
                    for grandchild in child.children:
                        if grandchild.type == 'identifier':
                            variables.append(grandchild)
                if child.type == 'reference_declarator':
                    for grandchild in child.children:
                        if grandchild.type == 'identifier':
                            variables.append(grandchild)
                if child.type == 'array_declarator':
                    for grandchild in child.children:
                        if grandchild.type == 'identifier':
                            variables.append(grandchild)

        for child in current_node.children:
            recursive_find(child)

    recursive_find(node)
    return variables

def replace_var_name(root_node, func_node, old_var, new_var):
    code_bytes = root_node.text
    variable_nodes = []

    def traverse(node):
        if node.type == 'identifier':
            variable_nodes.append(node)
        for child in node.children:
            traverse(child)

    traverse(func_node)

    replacements = []
    for node in variable_nodes:
        if node.text.decode('utf8') == old_var:
            start = node.start_byte
            end = node.end_byte
            assert code_bytes[start:end].decode('utf8') == old_var
            replacements.append((start, end))

    offset = 0
    for start, end in replacements:
        adj_start = start + offset
        adj_end = end + offset
        assert code_bytes[adj_start:adj_end].decode('utf8') == old_var
        code_bytes = code_bytes[:adj_start] + bytes(new_var, 'utf8') + code_bytes[adj_end:]
        offset += len(bytes(new_var, 'utf8')) - (end - start)

    code = code_bytes.decode('utf8')
    return code