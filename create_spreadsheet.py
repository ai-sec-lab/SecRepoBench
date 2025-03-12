import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
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

def clean_block(code_block):
    # Remove multi-line comments first
    code_block = re.sub(r'/\*[\s\S]*?\*/', '', code_block)
    
    # Split into lines
    code_block_lines = re.split('\n', code_block)
    
    # Process each line - remove single-line comments and strip whitespace
    code_block_lines_clean = []
    for ln in code_block_lines:
        # Remove single line comments
        ln = re.sub(r'//.*$', '', ln)
        # Strip whitespace
        ln = ln.strip()
        # Only add non-empty lines
        if ln:
            code_block_lines_clean.append(ln)
    
    # Join the clean lines
    code_block_clean = '\n'.join(code_block_lines_clean)
    
    return code_block_clean

def get_c_cpp_file(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        path = c_path
    elif os.path.exists(cpp_path):
        path = cpp_path
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return ""
    with open(path, 'r') as f:
        content = f.read()
    return content

def norm_distance(text_1, text_2):
    return distance(text_1, text_2) / max(len(text_1), len(text_2))

with open('filter_logs/cases.json', 'r') as f:
    cases = json.load(f)

with open('ids.txt', 'r') as f:
    ids = f.read().splitlines()[1:]

with open('id2var.json', 'r') as f:
    id2var = json.load(f)

table = {}

for id in ids:
    row = []

    project = cases[id]['project_name']
    row.append(project)

    changed_file = cases[id]['changed_file']
    row.append(changed_file)

    with open(f'ARVO-Meta/meta/{id}.json', 'r') as f:
        meta = json.load(f)
    fix_commit = meta['fix']
    row.append(fix_commit)

    # 'ground truth' code blocks
    sec_code_block_base = get_c_cpp_file(f'descriptions/{id}/sec_code_block_base')
    vul_code_block_base = get_c_cpp_file(f'descriptions/{id}/vul_code_block_base')
    sec_code_block_perturbed = get_c_cpp_file(f'descriptions/{id}/sec_code_block_perturbed')
    vul_code_block_perturbed = get_c_cpp_file(f'descriptions/{id}/vul_code_block_perturbed')

    # num lines in code blocks
    num_lns_sec_code_block_perturbed = len(re.findall(r'\n', sec_code_block_perturbed))
    num_lns_vul_code_block_perturbed = len(re.findall(r'\n', vul_code_block_perturbed))

    row.append(num_lns_sec_code_block_perturbed)
    row.append(num_lns_vul_code_block_perturbed)

    old_var = id2var[id]['old_var']
    new_var = id2var[id]['new_var']

    row.append(old_var)
    row.append(new_var)

    # Get descriptions, prompts
    description_local = f'descriptions/{id}/desc.txt'
    if os.path.exists(description_local):
        with open(f'descriptions/{id}/desc.txt') as f:
            description = f.read()
        row.append(description)
    else:
        row.append("")
        # row.append("")

    row.append(sec_code_block_base)
    row.append(vul_code_block_base)
    row.append(sec_code_block_perturbed)
    row.append(vul_code_block_perturbed)

    sonnett_code_block = f'completions/{id}/claude-3.5-sonnet-filled-code-func-sec-generic-perturbed_code_completion.txt'
    if os.path.exists(sonnett_code_block):
        with open(sonnett_code_block, 'r') as f:
            sonnett_code_block = f.read()
        row.append(sonnett_code_block)
    else:
        row.append("")

    clean_sec_code_block_base = clean_block(sec_code_block_base)
    clean_vul_code_block_base = clean_block(vul_code_block_base)
    clean_sec_code_block_perturbed = clean_block(sec_code_block_perturbed)
    clean_vul_code_block_perturbed = clean_block(vul_code_block_perturbed)

    clean_sonnett_code_block = clean_block(sonnett_code_block)

    edit_distance_sec_base_vs_gen = norm_distance(clean_sec_code_block_base, clean_sonnett_code_block)
    row.append(edit_distance_sec_base_vs_gen)

    edit_distance_vul_base_vs_gen = norm_distance(clean_vul_code_block_base, clean_sonnett_code_block)
    row.append(edit_distance_vul_base_vs_gen)

    edit_distance_sec_perturbed_vs_gen = norm_distance(clean_sec_code_block_perturbed, clean_sonnett_code_block)
    row.append(edit_distance_sec_perturbed_vs_gen)

    edit_distance_vul_peturbed_vs_gen = norm_distance(clean_vul_code_block_perturbed, clean_sonnett_code_block)
    row.append(edit_distance_vul_peturbed_vs_gen)

    # add to table
    table[id] = row


columns = [
    'project_name',
    'changed_file',
    'commit_url',
    'num_lines_sec_code_block',
    'num_lines_vul_code_block',
    'old_var',
    'new_var',
    'description',
    'sec_code_block_base',
    'vul_code_block_base',
    'sec_code_block_perturbed',
    'vul_code_block_perturbed',
    'sonnett_code_block_perturbed',
    'norm_edit_distance_sec_base_vs_generated',
    'norm_edit_distance_vul_base_vs_generated',
    'norm_edit_distance_sec_perturbed_vs_generated',
    'norm_edit_distance_vul_perturbed_vs_generated',
]

df = pd.DataFrame.from_dict(table, orient='index', columns=columns)
df.to_excel('summary_table.xlsx')

sns.set_style("whitegrid")
plt.figure(figsize=(12, 6))

ax = sns.boxplot(data=df[[
    'norm_edit_distance_sec_base_vs_generated',
    'norm_edit_distance_vul_base_vs_generated',
    'norm_edit_distance_sec_perturbed_vs_generated',
    'norm_edit_distance_vul_perturbed_vs_generated'
]])

plt.title('Comparison of Normalized Edit Distances', fontsize=16)
plt.ylabel('Normalized Edit Distance', fontsize=14)
plt.xticks(rotation=45, ha='right', fontsize=12)

ax.set_xticklabels([
    'Secure Base vs Generated',
    'Vulnerable Base vs Generated',
    'Secure Perturbed vs Generated',
    'Vulnerable Perturbed vs Generated'
])

plt.grid(axis='y', linestyle='--', alpha=0.7)

plt.tight_layout()

plt.show()

plt.savefig('edit_distance_comparison.png', dpi=300, bbox_inches='tight')