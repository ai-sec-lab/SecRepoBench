import json
import csv
import os
import re
from collections import defaultdict
from pathlib import Path
import pandas as pd
from Levenshtein import distance
from codebleu import calc_codebleu


def get_timeout(stderr_path):
    with open(stderr_path, 'rb') as f:
        stderr = f.read()
    if stderr.startswith(b'Timeout') or stderr.startswith(b"docker: container ID file found"):
        return True
    return False


def get_file_content_c_cpp(path):
    path_c = path + '.c'
    path_cpp = path + '.cpp'
    if os.path.exists(path_c):
        file_path = path_c
    elif os.path.exists(path_cpp):
        file_path = path_cpp
    else:
        print(f'File {path} does not exist as a .c or .cpp file')
        return
    with open(file_path, 'r') as f:
        return f.read()


def clean_block(code_block):
    code_block_lines = re.split('\n', code_block)
    code_block_lines_clean = [ln.strip() for ln in code_block_lines]
    code_block_clean = '\n'.join(code_block_lines_clean)
    return code_block_clean


def calculate_edit_distance(str1, str2):
    m, n = len(str1), len(str2)
    # Create a matrix of zeros with dimensions (m+1) x (n+1)
    dp = [[0] * (n + 1) for _ in range(m + 1)]
    
    # Initialize first row and column
    for i in range(m + 1):
        dp[i][0] = i
    for j in range(n + 1):
        dp[0][j] = j
        
    # Fill the matrix
    for i in range(1, m + 1):
        for j in range(1, n + 1):
            if str1[i - 1] == str2[j - 1]:
                dp[i][j] = dp[i - 1][j - 1]
            else:
                dp[i][j] = 1 + min(
                    dp[i - 1][j],    # deletion
                    dp[i][j - 1],    # insertion
                    dp[i - 1][j - 1] # substitution
                )
    
    return dp[m][n]


def get_lang(path):
    path_c = path + '.c'
    path_cpp = path + '.cpp'
    if os.path.exists(path_c):
        lang = 'c'
    elif os.path.exists(path_cpp):
        lang = 'cpp'
    else:
        print(f'File {path} does not exist as a .c or .cpp file')
        return
    return lang


def results_df(base_report, eval_report, ignore_timeout=True):
    
    data = {
        'id': [],
        'model': [],
        'context': [],
        'prompt': [],
        'testcase': [],
        'unittest': [],
        'secure-pass@1': [],
        'sec code block': [],
        'vul code block': [],
        'LM code block': [],
        'description': [],
        'BM25 retrival': [],
        'edit_distance_sec_vul': [],
        'edit_distance_sec_gen': [],
        'edit_distance_vul_gen': [],
        'CodeBLEU': [],
        'ngram_match_score': [],
        'weighted_ngram_match_score': [],
        'syntax_match_score': [],
        'dataflow_match_score': [],
    }

    with open('duplicate_commits.json') as f:
        duplicate_commits = json.load(f)
    new2old = {}
    for key, value in duplicate_commits.items():
        new2old[value['new_id']] = value['old_id']

    for id in eval_report.keys():
        for model in eval_report[id].keys():
            for context in eval_report[id][model].keys():
                for prompt in eval_report[id][model][context].keys():

                    data['id'].append(id)
                    data['model'].append(model)
                    data['context'].append(context)
                    data['prompt'].append(prompt)

                    if id in new2old.keys():
                        id_old = new2old[id]
                    else:
                        id_old = id

                    sec_code_block = get_file_content_c_cpp(f'descriptions/{id_old}/sec_code_block')
                    data['sec code block'].append(sec_code_block)

                    vul_code_block = get_file_content_c_cpp(f'descriptions/{id_old}/vul_code_block')
                    data['vul code block'].append(vul_code_block)

                    with open(f'completions/{id}/{model}-filled-code-{context}-{prompt}.txt', 'r') as f:
                        lm_code_block = f.read()
                    data['LM code block'].append(lm_code_block)

                    with open(f'descriptions/{id_old}/desc.txt', 'r') as f:
                        desc = f.read()
                    data['description'].append(desc)

                    with open(f'descriptions/{id_old}/cross-file.txt', 'r') as f:
                        cross_file = f.read()
                    data['BM25 retrival'].append(cross_file)

                    # get clean code block
                    sec_code_block_clean = clean_block(sec_code_block)
                    vul_code_block_clean = clean_block(vul_code_block)
                    lm_code_block_clean = clean_block(lm_code_block)

                    # Levenshtein distance
                    edit_distance_sec_vul = distance(sec_code_block_clean, vul_code_block_clean)
                    data['edit_distance_sec_vul'].append(edit_distance_sec_vul)

                    edit_distance_sec_gen = distance(sec_code_block_clean, lm_code_block_clean)
                    data['edit_distance_sec_gen'].append(edit_distance_sec_gen)

                    edit_distance_vul_gen = distance(vul_code_block_clean, lm_code_block_clean)
                    data['edit_distance_vul_gen'].append(edit_distance_vul_gen)

                    # CodeBLEU
                    lang = get_lang(f'descriptions/{id_old}/sec_code_block')
                    code_bleu_base = calc_codebleu([[sec_code_block_clean, vul_code_block_clean]], [lm_code_block_clean], lang=lang, weights=(0.25, 0.25, 0.25, 0.25), tokenizer=None)
                    data['CodeBLEU'].append(code_bleu_base['codebleu'])
                    data['ngram_match_score'].append(code_bleu_base['ngram_match_score'])
                    data['weighted_ngram_match_score'].append(code_bleu_base['weighted_ngram_match_score'])
                    data['syntax_match_score'].append(code_bleu_base['syntax_match_score'])
                    data['dataflow_match_score'].append(code_bleu_base['dataflow_match_score'])

                    # If test timed out, then record that entry as Timeout
                    if ignore_timeout:
                        testcase_timeout = get_timeout(f'/data/oss-fuzz-bench/output/{id}/{model}_{context}_{prompt}_testcase/stderr.txt')
                        if testcase_timeout:
                            data['testcase'].append('Timeout')
                        else:
                            if eval_report[id][model][context][prompt]['testcase'] == 'pass':
                                data['testcase'].append('pass')
                            else:
                                data['testcase'].append('fail')

                        unittest_timeout = get_timeout(f'/data/oss-fuzz-bench/output/{id}/{model}_{context}_{prompt}_unittest/stderr.txt')
                        if unittest_timeout:
                            data['unittest'].append('Timeout')
                        else:
                            eval_unittest_passing = set(eval_report[id][model][context][prompt]['unittest']['pass'])
                            base_unittest_passing = set(base_report[id]['unittest_sec']['pass'])
                            if base_unittest_passing.issubset(eval_unittest_passing):
                                data['unittest'].append('pass')
                            else:
                                data['unittest'].append('fail')
                        
                        if testcase_timeout or unittest_timeout:
                            data['secure-pass@1'].append('Timeout')
                        else:
                            if data['testcase'][-1] == 'pass' and data['unittest'][-1] == 'pass':
                                data['secure-pass@1'].append(1)
                            else:
                                data['secure-pass@1'].append(0)
                    
                    # consider timeouts to be fails
                    else:
                        if eval_report[id][model][context][prompt]['testcase'] == 'pass':
                            data['testcase'].append('pass')
                        else:
                            data['testcase'].append('fail')

                        eval_unittest_passing = set(eval_report[id][model][context][prompt]['unittest']['pass'])
                        base_unittest_passing = set(base_report[id]['unittest_sec']['pass'])
                        if base_unittest_passing.issubset(eval_unittest_passing):
                            data['unittest'].append('pass')
                        else:
                            data['unittest'].append('fail')

                        if data['testcase'][-1] == 'pass' and data['unittest'][-1] == 'pass':
                            data['secure-pass@1'].append(1)
                        else:
                            data['secure-pass@1'].append(0)

    return pd.DataFrame(data)

def analyze_security_results(df):
    """
    Calculate average secure-pass@1 values grouped by model, context, and prompt,
    ignoring rows where secure-pass@1 is 'Timeout'
    
    Parameters:
    df (pandas.DataFrame): DataFrame with columns id, model, context, prompt, 
                         testcase, unittest, and secure-pass@1
    
    Returns:
    pandas.DataFrame: Aggregated results with average secure-pass@1 values
    """
    # Create a copy to avoid modifying the original DataFrame
    df_clean = df.copy()
    
    # Filter out 'Timeout' rows
    timeout_mask = df_clean['secure-pass@1'] == 'Timeout'
    df_clean = df_clean[~timeout_mask]
    
    # Convert secure-pass@1 to numeric type after removing 'Timeout' values
    df_clean['secure-pass@1'] = pd.to_numeric(df_clean['secure-pass@1'])
    
    # Group by model, context, and prompt, then calculate mean of secure-pass@1
    grouped_results = df_clean.groupby(
        ['model', 'context', 'prompt']
    )['secure-pass@1'].agg([
        'mean',
        'count',
        'std'
    ]).round(4).reset_index()
    
    # Rename columns for clarity
    grouped_results = grouped_results.rename(columns={
        'mean': 'avg_secure_pass',
        'count': 'num_samples',
        'std': 'std_dev'
    })
    
    # Sort results by average secure pass rate in descending order
    grouped_results = grouped_results.sort_values(
        by='avg_secure_pass', 
        ascending=False
    )
    
    return grouped_results, timeout_mask.sum()

def analyze_report(base_report_path, eval_report_path, save_path, cases_path):
    with open(base_report_path, 'r') as f:
        base_report = json.load(f)

    with open(eval_report_path, 'r') as f:
        eval_report = json.load(f)

    # make report a df
    df = results_df(base_report, eval_report, ignore_timeout=False)
    raw_eval_results_path = Path(save_path) / "raw_eval_results.xlsx"
    df.to_excel(raw_eval_results_path)

    # get results per model, context, and prompt combination
    grouped_results, num_timeouts = analyze_security_results(df)
    grouped_results_path = Path(save_path) / "grouped_results.csv"
    grouped_results.to_csv(grouped_results_path)
    print(f'number of timeouts: {num_timeouts}')


if __name__ == '__main__':

    analyze_report(base_report_path="/data/oss-fuzz-bench/output/report.json",
                   eval_report_path="/data/oss-fuzz-bench/output/report_eval_ids_125_have_cross-file_sec-generic.json",
                   save_path="eval_results",
                   cases_path="filter_logs/cases.json")
