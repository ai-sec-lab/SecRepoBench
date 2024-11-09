import sys
import os
import re
import csv
from tree_sitter import Language, Parser
from utils import *

def replace_code_block_with_mask(source_code, code_block):
    # Replaces the code in the given range with "// <MASK>"
    if isinstance(code_block, list):
        start_byte = code_block[0].start_byte
        end_byte = code_block[-1].end_byte
    else:
        # Check if the code_block is a 'compound_statement' and adjust accordingly
        if code_block.type == 'compound_statement':
            # Exclude the braces
            children = code_block.children
            if len(children) >= 2:
                # Assume first child is '{', last child is '}'
                start_byte = children[0].end_byte  # End of '{'
                end_byte = children[-1].start_byte  # Start of '}'
                # Handle empty function bodies
                if start_byte >= end_byte:
                    # Empty body, nothing to mask
                    return source_code
            else:
                # No braces found, treat as empty
                return source_code
        else:
            start_byte = code_block.start_byte
            end_byte = code_block.end_byte

    # Replace the block with "// <MASK>"
    masked_code = source_code[:start_byte] + "// <MASK>" + source_code[end_byte:]
    return masked_code

def main(id):
    # if len(sys.argv) != 2:
    #     print("Usage: python script.py xxx.diff modified_file")
    #     sys.exit(1)

    # id = sys.argv[1]
    print(f"Processing {id}")
    diff_file = f'/home/yanjun/ethan/arvo-oss-fuzz-bench/ARVO-Meta/patches/{id}.diff'
    source_file = f'/data/fyj-oss-fuzz-bench/{id}/patches/sec.txt'
    destination_file = f'/data/fyj-oss-fuzz-bench/{id}/patches/mask.txt'
    if os.path.exists(f'/data/fyj-oss-fuzz-bench/{id}/patches/err_empty.txt'):
        os.remove(f'/data/fyj-oss-fuzz-bench/{id}/patches/err_empty.txt')

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

    if function_node is None:
        print("No modified lines found within any function.")
        sys.exit(1)

    # Calculate x-3 and y+3 if x == y
    if x == y or int(id) in [1975, 2870, 7201, 10731, 20333, 24436, 28484, 31789, 43209, 44483, 49068, 53483, 54625, 64849]:
        x = max(1, x - 3)
        y = y + 3

    # Find the natural code block
    code_block = find_code_block(function_node, x, y, total_lines)

    # Replace the code block with a non-op comment "// <MASK>"
    if code_block is None:
        print("Could not find a suitable code block.")
        sys.exit(1)
    else:
        # Replace the identified block with the comment
        modified_source_code = replace_code_block_with_mask(source_code, code_block)

        # Write the modified source code back to the file (or write to a new file)
        with open(destination_file, 'w') as f:
            f.write(modified_source_code)
        print(f"Code block replaced with // <MASK> in {source_file}")

if __name__ == "__main__":
    with open('ids/print_checked_cases.txt', 'r') as f:
        ids = f.readlines()
        ids = [int(id.strip()) for id in ids]

    for id in ids:
        main(id)