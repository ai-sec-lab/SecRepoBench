import sys
import os
import re
import csv
from tree_sitter import Language, Parser
import random
import json
from utils import *
from get_new_var import APIEvaler, get_new_var


def get_c_cpp_file(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        path = c_path
    elif os.path.exists(cpp_path):
        path = cpp_path
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return
    with open(path, 'r') as f:
        content = f.read()
    return content


def get_language(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        LANGUAGE = C_LANGUAGE
    elif os.path.exists(cpp_path):
        LANGUAGE = CPP_LANGUAGE
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return
    return LANGUAGE


def get_extension(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        ext = 'c'
    elif os.path.exists(cpp_path):
        ext = 'cpp'
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return
    return ext


def get_mask_line(func):
    func_lines = re.split(r'\n', func)
    for ln_num, ln in enumerate(func_lines):
        if "// <MASK>" in ln:
            return ln_num + 1


def replace(id, evaler):
    print(f"Processing {id}")

    # set ts language
    LANGUAGE = get_language(f"descriptions/{id}/mask_base")

    # get extension
    ext = get_extension(f"descriptions/{id}/mask_base")

    # get id2var
    with open('id2var.json', 'r') as f:
        id2var = json.load(f)

    # load in non-perturbed files
    mask_base = get_c_cpp_file(f"descriptions/{id}/mask_base")
    mask_sec_func_base = get_c_cpp_file(f"descriptions/{id}/mask_sec_func_base")
    sec_code_block_base = get_c_cpp_file(f"descriptions/{id}/sec_code_block_base")
    vul_code_block_base = get_c_cpp_file(f"descriptions/{id}/vul_code_block_base")
    sec_func_base = get_c_cpp_file(f"descriptions/{id}/sec_func_base")

    # parse each with tree-sitter
    parser = Parser(LANGUAGE)
    ts_mask = parser.parse(bytes(mask_base, 'utf8'))
    ts_mask_sec_func = parser.parse(bytes(mask_sec_func_base, 'utf8'))
    ts_sec_code_block = parser.parse(bytes(sec_code_block_base, 'utf8'))
    ts_vul_code_block = parser.parse(bytes(vul_code_block_base, 'utf8'))
    ts_sec_func = parser.parse(bytes(sec_func_base, 'utf8'))

    # get all variables in the function body
    vars_all = find_all_variables(ts_sec_func.root_node)

    # get variables in function body before the code blocks
    mask_line = get_mask_line(mask_sec_func_base)
    vars_before_mask = find_variables_before_code_block(ts_sec_func.root_node, mask_line)

    # get variables in sec code block, vul code block -- these are already in vars_before_mask
    vars_sec_code_block = find_variables_in_code_block(ts_sec_code_block.root_node, vars_before_mask)
    vars_vul_code_block = find_variables_in_code_block(ts_vul_code_block.root_node, vars_before_mask)

    # Select a local variable to be replaced
    vars_both_code_blocks = list(set(vars_sec_code_block).intersection(set(vars_vul_code_block)))
    vars_either_code_blocks = list(set(vars_sec_code_block).union(set(vars_vul_code_block)))
    # case 1: local var in sec code block, vul code block, and in the function body before the code block
    if len(vars_both_code_blocks) > 0:
        print(f"ID {id}: case 1")
        vars_choices = vars_both_code_blocks

    # case 2:  no local variable in both code blocks, select a local variable that is in at least one code block and before the function body
    elif len(vars_either_code_blocks) > 0:
        print(f"ID {id}: case 2")
        vars_choices = vars_either_code_blocks

    # case 3: no local variable in either code block, then select a local variable that is in the function body before the code blocks
    elif len(vars_before_mask) > 0:
        print(f"ID {id}: case 3")
        vars_choices = vars_before_mask

    # case 4: no local variables, something is probably wrong
    else:
        print(f"ID {id}: case 4")
        return

    # Select old var, new var
    # reuse id2var.json if applicable
    old_var = None
    new_var = None
    if id in id2var.keys():
        old_var = id2var[id]['old_var']
        new_var = id2var[id]['new_var']

    if old_var not in vars_choices or new_var in vars_all:
        old_var = random.choice(vars_choices)
        new_var = get_new_var(old_var, mask_sec_func_base, evaler, vars_all)

    # replace var in non-masked files
    sec_code_block_per = replace_var_name(ts_sec_code_block, old_var, new_var)
    vul_code_block_per = replace_var_name(ts_vul_code_block, old_var, new_var)
    sec_func_per = replace_var_name(ts_sec_func, old_var, new_var)

    # replace var in masked files
    if sec_func_per.count(sec_code_block_per) == 1:
        mask_sec_func_per = sec_func_per.replace(sec_code_block_per, "// <MASK>")
    else:
        mask_sec_func_per = ""
        print(f"Id {id}: sec_code_block_per does not occur once in mask_sec_func_per")

    if mask_base.count(mask_sec_func_base) == 1:
        mask_per = mask_base.replace(mask_sec_func_base, mask_sec_func_per)
    else:
        mask_per = ""
        print(f"Id {id}: mask_sec_func_base does not occur once in mask_base")

    # write the perturbed files
    with open(f"descriptions/{id}/sec_code_block_perturbed.{ext}", 'w') as f:
        f.write(sec_code_block_per)

    with open(f"descriptions/{id}/vul_code_block_perturbed.{ext}", 'w') as f:
        f.write(vul_code_block_per)
    
    with open(f"descriptions/{id}/sec_func_perturbed.{ext}", 'w') as f:
        f.write(sec_func_per)
    
    with open(f"descriptions/{id}/mask_sec_func_perturbed.{ext}", 'w') as f:
        f.write(mask_sec_func_per)
    
    with open(f"descriptions/{id}/mask_perturbed.{ext}", 'w') as f:
        f.write(mask_per)

    # save id2var
    id2var[id] = {
        'old_var': old_var,
        'new_var': new_var
    }
    with open('id2var.json', 'w') as f:
        json.dump(id2var, f, indent=4)


if __name__ == "__main__":
    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    evaler = APIEvaler()

    for id in ids:
        replace(id, evaler)
