import json
import pandas as pd
import os
import requests
from Levenshtein import distance
import re


def check_link_exists(url):
    try:
        response = requests.head(url, allow_redirects=True, timeout=5)
        return response.status_code == 200
    except requests.RequestException:
        return False


with open('/home/cdilgren/project_benchmark/filter_logs/cases.json', 'r') as f:
    cases = json.load(f)


# with open('analyze_report/ids_each_step.json', 'r') as f:
#     ids_each_step = json.load(f)
# ids_pass_testcase_unittest = ids_each_step['ids_pass_testcase_unittest']

with open('ids_125_have_good.txt', 'r') as f:
    ids = f.read().splitlines()[1:]

for id in ids:
    # sec_code_block_base is the 'ground truth' for the non-perturbed version
    sec_code_block_base_file_c = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_base.c'
    sec_code_block_base_file_cpp = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_base.cpp'
    if os.path.exists(sec_code_block_base_file_c):
        with open(sec_code_block_base_file_c, 'r') as f:
            sec_code_block_base = f.read().splitlines()
    elif os.path.exists(sec_code_block_base_file_cpp):
        with open(sec_code_block_base_file_cpp, 'r') as f:
            sec_code_block_base = f.read().splitlines()

    # sec_code_block_perturbed is the 'ground truth' for the perturbed version
    sec_code_block_perturbed_file_c = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_perturbed.c'
    sec_code_block_perturbed_file_cpp = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_perturbed.cpp'
    if os.path.exists(sec_code_block_perturbed_file_c):
        with open(sec_code_block_perturbed_file_c, 'r') as f:
            sec_code_block_perturbed = f.read().splitlines()
    elif os.path.exists(sec_code_block_perturbed_file_cpp):
        with open(sec_code_block_perturbed_file_cpp, 'r') as f:
            sec_code_block_perturbed = f.read().splitlines()

    # sonnett_code_block_base is the sonnett generated code for the non-perturbed version
    sonnett_code_block_base_local = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_code_completion.txt'
    if os.path.exists(sonnett_code_block_base_local):
        with open(sonnett_code_block_base_local, 'r') as f:
            sonnett_code_block_base = f.read()

    # sonnett_code_block_perturbed is the sonnet generated code for the perturbed version
    sonnett_code_block_perturbed_local = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-perturbed_code_completion.txt'
    if os.path.exists(sonnett_code_block_perturbed_local):
        with open(sonnett_code_block_perturbed_local, 'r') as f:
            sonnett_code_block_perturbed = f.read()



def clean_block(code_block):
    code_block_lines = re.split('\n', code_block)
    code_block_lines_clean = [ln.strip() for ln in code_block_lines]
    code_block_clean = '\n'.join(code_block_lines_clean)
    return code_block_clean


table = {}

for id in ids:
    row = []

    project = cases[id]['project_name']
    row.append(project)

    changed_file = cases[id]['changed_file']
    row.append(changed_file)

    with open(f'/home/cdilgren/project_benchmark/ARVO-Meta/meta/{id}.json', 'r') as f:
        meta = json.load(f)
    fix_commit = meta['fix']
    row.append(fix_commit)

    # num lines in code blocks
    sec_code_block_base_file_c = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_base.c'
    sec_code_block_base_file_cpp = f'/home/cdilgren/project_benchmark/descriptions/{id}/sec_code_block_base.cpp'
    if os.path.exists(sec_code_block_base_file_c):
        ext = 'c'
        with open(sec_code_block_base_file_c, 'r') as f:
            sec_code_block_base = f.read().splitlines()
        num_lns_sec_code_block_base = len(sec_code_block_base)
        row.append(num_lns_sec_code_block_base)
    elif os.path.exists(sec_code_block_base_file_cpp):
        ext = 'cpp'
        with open(sec_code_block_base_file_cpp, 'r') as f:
            sec_code_block_base = f.read().splitlines()
        num_lns_sec_code_block_base = len(sec_code_block_base)
        row.append(num_lns_sec_code_block_base)
    else:
        row.append("")

    vul_code_block_base_file_c = f'/home/cdilgren/project_benchmark/descriptions/{id}/vul_code_block_base.c'
    vul_code_block_base_file_cpp = f'/home/cdilgren/project_benchmark/descriptions/{id}/vul_code_block_base.cpp'
    if os.path.exists(vul_code_block_base_file_c):
        with open(vul_code_block_base_file_c, 'r') as f:
            vul_code_block_base = f.read().splitlines()
        num_lns_vul_code_block_base = len(vul_code_block_base)
        row.append(num_lns_vul_code_block_base)
    elif os.path.exists(vul_code_block_base_file_cpp):
        with open(vul_code_block_base_file_cpp, 'r') as f:
            vul_code_block_base = f.read().splitlines()
        num_lns_vul_code_block_base = len(vul_code_block_base)
        row.append(num_lns_vul_code_block_base)
    else:
        row.append("")

    # Get base code blocks, masks
    sec_code_block_base_local = f'descriptions/{id}/sec_code_block_base.{ext}'
    if os.path.exists(sec_code_block_base_local):
        with open(sec_code_block_base_local) as f:
            sec_code_block_base = f.read()
        row.append(sec_code_block_base)
        # sec_code_block_base_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/sec_code_block_base.{ext}'
        # row.append(sec_code_block_base_link)
    else:
        row.append("")

    # vul_code_block_base_local = f'descriptions/{id}/vul_code_block_base.{ext}'
    # if os.path.exists(vul_code_block_base_local):
    #     vul_code_block_base_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/vul_code_block_base.{ext}'
    #     row.append(vul_code_block_base_link)
    # else:
    #     row.append("")

    # function_mask_base_local = f'descriptions/{id}/mask_sec_func_base.{ext}'
    # if os.path.exists(function_mask_base_local):
    #     function_mask_base_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/mask_sec_func_base.{ext}'
    #     row.append(function_mask_base_link)
    # else:
    #     row.append("")

    # file_mask_base_local = f'descriptions/{id}/mask_base.{ext}'
    # if os.path.exists(file_mask_base_local):
    #     file_mask_base_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/mask_base.{ext}'
    #     row.append(file_mask_base_link)
    # else:
    #     row.append("")

    # Get perturbed code blocks, masks
    sec_code_block_perturbed_local = f'descriptions/{id}/sec_code_block_perturbed.{ext}'
    if os.path.exists(sec_code_block_perturbed_local):
        with open(sec_code_block_perturbed_local) as f:
            sec_code_block_perturbed = f.read()
        row.append(sec_code_block_perturbed)
        # sec_code_block_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/sec_code_block_perturbed.{ext}'
        # row.append(sec_code_block_perturbed_link)
    else:
        row.append("")

    # vul_code_block_perturbed_local = f'descriptions/{id}/vul_code_block_perturbed.{ext}'
    # if os.path.exists(vul_code_block_perturbed_local):
    #     vul_code_block_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/vul_code_block_perturbed.{ext}'
    #     row.append(vul_code_block_perturbed_link)
    # else:
    #     row.append("")

    # function_mask_perturbed_local = f'descriptions/{id}/mask_sec_func_perturbed.{ext}'
    # if os.path.exists(function_mask_perturbed_local):
    #     function_mask_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/mask_sec_func_perturbed.{ext}'
    #     row.append(function_mask_perturbed_link)
    # else:
    #     row.append("")

    # file_mask_perturbed_local = f'descriptions/{id}/mask_perturbed.{ext}'
    # if os.path.exists(file_mask_perturbed_local):
    #     file_mask_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/mask_perturbed.{ext}'
    #     row.append(file_mask_perturbed_link)
    # else:
    #     row.append("")

    # Get descriptions, prompts
    description_local = f'descriptions/{id}/desc.txt'
    if os.path.exists(description_local):
        with open(f'/home/cdilgren/project_benchmark/descriptions/{id}/desc.txt') as f:
            description = f.read()
        row.append(description)
        # description_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/desc.txt'
        # row.append(description_link)
    else:
        row.append("")
        # row.append("")

    prompt_desc_local = f'descriptions/{id}/desc_prompt.txt'
    if os.path.exists(prompt_desc_local):
        prompt_desc_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/desc_prompt.txt'
        row.append(prompt_desc_link)
    else:
        row.append("")

    # BM25_retrivial_local = f'descriptions/{id}/cross-file.txt'
    # if os.path.exists(BM25_retrivial_local):
    #     BM25_retrivial_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/cross-file.txt'
    #     row.append(BM25_retrivial_link)
    # else:
    #     row.append("")

    # system_prompt_code_gen = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_system_prompt.txt'
    # if os.path.exists(system_prompt_code_gen):
    #     system_prompt_code_gen_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_system_prompt.txt'
    #     row.append(system_prompt_code_gen_link)
    # else:
    #     row.append("")

    # Get the prompt(s) used to do code completions
    prompt_code_gen_base = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_prompt.txt'
    if os.path.exists(prompt_code_gen_base):
        prompt_code_gen_link_base = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_prompt.txt'
        row.append(prompt_code_gen_link_base)
    else:
        row.append("")

    sonnett_code_block_base_local = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_code_completion.txt'
    if os.path.exists(sonnett_code_block_base_local):
        with open(sonnett_code_block_base_local, 'r') as f:
            sonnett_code_block_base = f.read()
        row.append(sonnett_code_block_base)
        # sonnett_code_block_base_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_code_completion.txt'
        # row.append(sonnett_code_block_base_link)
    else:
        row.append("")

    sonnett_code_block_perturbed_local = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-perturbed_code_completion.txt'
    if os.path.exists(sonnett_code_block_perturbed_local):
        with open(sonnett_code_block_perturbed_local, 'r') as f:
            sonnett_code_block_perturbed = f.read()
        row.append(sonnett_code_block_perturbed)
        # sonnett_code_block_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-perturbed_code_completion.txt'
        # row.append(sonnett_code_block_perturbed_link)
    else:
        row.append("")

    sonnett_code_block_base_no_desc_local = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-base_no_desc_code_completion.txt'
    if os.path.exists(sonnett_code_block_base_no_desc_local):
        with open(sonnett_code_block_base_no_desc_local, 'r') as f:
            sonnett_code_block_base_no_desc = f.read()
        row.append(sonnett_code_block_base_no_desc)
        # sonnett_code_block_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-perturbed_code_completion.txt'
        # row.append(sonnett_code_block_perturbed_link)
    else:
        row.append("")

    sonnett_code_block_perturbed_no_desc_local = f'/home/cdilgren/project_benchmark/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-perturbed_no_desc_code_completion.txt'
    if os.path.exists(sonnett_code_block_perturbed_no_desc_local):
        with open(sonnett_code_block_perturbed_no_desc_local, 'r') as f:
            sonnett_code_block_perturbed_no_desc = f.read()
        row.append(sonnett_code_block_perturbed_no_desc)
        # sonnett_code_block_perturbed_link = f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/completions/{id}/claude-3.5-sonnet-filled-code-cross-file-sec-generic-perturbed_code_completion.txt'
        # row.append(sonnett_code_block_perturbed_link)
    else:
        row.append("")

    clean_sec_code_block_base = clean_block(sec_code_block_base)
    clean_sec_code_block_perturbed = clean_block(sec_code_block_perturbed)

    clean_sonnett_code_block_base = clean_block(sonnett_code_block_base)
    clean_sonnett_code_block_perturbed = clean_block(sonnett_code_block_perturbed)

    clean_sonnett_code_block_base_no_desc = clean_block(sonnett_code_block_base_no_desc)
    clean_sonnett_code_block_perturbed_no_desc = clean_block(sonnett_code_block_perturbed_no_desc)

    edit_distance_base = distance(clean_sonnett_code_block_base, clean_sec_code_block_base)
    row.append(edit_distance_base)

    edit_distance_peturbed = distance(clean_sonnett_code_block_perturbed, clean_sec_code_block_perturbed)
    row.append(edit_distance_peturbed)

    edit_distance_two_gens = distance(clean_sonnett_code_block_base, clean_sonnett_code_block_perturbed)
    row.append(edit_distance_two_gens)

    edit_distance_base_no_desc = distance(clean_sonnett_code_block_base_no_desc, clean_sec_code_block_base)
    row.append(edit_distance_base_no_desc)

    edit_distance_perturbed_no_desc = distance(clean_sonnett_code_block_perturbed_no_desc, clean_sec_code_block_perturbed)
    row.append(edit_distance_perturbed_no_desc)

    # add to table
    table[id] = row


columns = [
    'project_name',
    'changed_file',
    'commit_url',
    'num_lines_sec_code_block',
    'num_lines_vul_code_block',
    'sec_code_block_base',
    'sec_code_block_perturbed',
    'description',
    'prompt_desc_link',
    'prompt_code_gen_link_base',
    'sonnett_code_block_base',
    'sonnett_code_block_perturbed',
    'sonnett_code_block_base_no_desc',
    'sonnett_code_block_perturbed_no_desc',
    'edit_distance_base',
    'edit_distance_perturbed',
    'edit_distance_code_gen_base_perturbed',
    'edit_distance_base_no_desc',
    'edit_distance_perturbed_no_desc']

df = pd.DataFrame.from_dict(table, orient='index', columns=columns)
df.to_excel('summary_table.xlsx')
