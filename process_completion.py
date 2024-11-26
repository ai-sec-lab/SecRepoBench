import sys
import os
import re
import csv
from tree_sitter import Language, Parser
import random
import json
from utils import *

def is_valid_function(code_text, parser):
    """
    Check if the given text can be parsed as a valid C/C++ function.
    
    Args:
        code_text (str): The code text to validate
        parser: Initialized tree-sitter parser
    
    Returns:
        tuple: (bool, str, dict) - (is_valid, error_message, function_info)
    """
    try:
        tree = parser.parse(bytes(code_text, "utf8"))
        root_node = tree.root_node
        
            
        # Get the first declaration
        function_node = None
        for node in root_node.children:
            if node.type in ["function_definition"]:
                function_node = node
                break
                
        if not function_node:
            return False, "No function definition or declaration found", {}
            
        # Extract function information
        function_info = {}
        
        # Find return type and function name
        for child in function_node.children:
            if child.type == "function_declarator":
                # Get function name
                for subchild in child.children:
                    if subchild.type == "identifier":
                        function_info['name'] = code_text[subchild.start_byte:subchild.end_byte]
                    elif subchild.type == "parameter_list":
                        function_info['has_parameters'] = len(subchild.children) > 2  # more than just parentheses
            elif child.type in ["primitive_type", "type_identifier"]:
                function_info['return_type'] = code_text[child.start_byte:child.end_byte]
                
        if function_node.type == "function_definition":
            function_info['has_body'] = True
            return True, "Valid function definition", function_info
        else:
            function_info['has_body'] = False
            return True, "Valid function declaration", function_info
            
    except Exception as e:
        return False, f"Error during parsing: {str(e)}", {}
    
def main(id):
    print(f"Processing {id}")
    diff_file = f'/home/yanjun/ethan/arvo-oss-fuzz-bench/ARVO-Meta/patches/{id}.diff'
    target_file, modified_lines = parse_diff_file(diff_file)
    file_extension = get_file_extension(target_file)
    language = determine_language(file_extension)
    LANGUAGE = C_LANGUAGE if language == 'c' else CPP_LANGUAGE

    print(f"Target file: {target_file}")
    print(language)

    # Parse the source code with Tree-sitter
    parser = Parser()
    parser.set_language(LANGUAGE)

    for file in os.listdir(f'completions/{id}'):
        if file.endswith('.txt'):
            with open(f'completions/{id}/{file}', 'r') as f:
                source_code = f.read()
            is_valid, error_message, function_info = is_valid_function(source_code, parser)
            if is_valid:
                print(file)
                start = source_code.find('{')
                end = source_code.rfind('}')
                # assert start != -1 and end != -1
                function_body = source_code[start+1:end]
                with open(f'completions/{id}/{file}', 'w') as f:
                    f.write(function_body)

if __name__ == '__main__':
    with open('/home/yanjun/arvo-oss-fuzz-bench/ids/final_ids.txt', 'r') as f:
        ids = f.read().splitlines()

    ids = sorted([int(id) for id in ids])
    for id in ids:
        main(id)