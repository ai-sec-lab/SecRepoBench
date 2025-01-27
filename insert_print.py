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
from pathlib import Path


def insert_print(id):
    # makes sec file but with printf statement immediately above code block
    # get mask file base
    mask_file_c = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_perturbed.c'
    mask_file_cpp = f'/home/cdilgren/project_benchmark/descriptions/{id}/mask_perturbed.cpp'
    if Path(mask_file_c).exists():
        mask_file = mask_file_c
    elif Path(mask_file_cpp).exists():
        mask_file = mask_file_cpp
    else:
        print(f'ID {id}: mask_perturbed file is missing in /home/cdilgren/project_benchmark/descriptions/{id}')
        return
    
    with open(mask_file, 'r') as f:
        sec_mask_content = f.read()

    # get sec code block
    sec_code_block_file_c = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_perturbed.c'
    sec_code_block_file_cpp = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_perturbed.cpp'
    if Path(sec_code_block_file_c).exists():
        sec_code_block_file = sec_code_block_file_c
    elif Path(sec_code_block_file_cpp).exists():
        sec_code_block_file = sec_code_block_file_cpp
    else:
        print(f'ID {id}: sec_code_block_perturbed file is missing in /home/cdilgren/project_benchmark/descriptions/{id}')
        return
    
    with open(sec_code_block_file, 'r') as f:
        sec_code_block = f.read()

    # add test printf statement after sec_code_block
    sec_code_block_print = sec_code_block + '\nprintf("This is a test for CodeGuard+\n"'

    # create mod file (sec file base with the LM patch)
    mod_file_content = sec_mask_content.replace("// <MASK>", sec_code_block_print)
    
    return mod_file_content
