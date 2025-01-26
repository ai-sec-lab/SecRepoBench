import json
import csv
from collections import defaultdict
from pathlib import Path
import pandas as pd
import os
from Levenshtein import distance
import re


def get_timeout(stderr_path):
    with open(stderr_path, 'rb') as f:
        stderr = f.read()
    if stderr.startswith(b'Timeout') or stderr.startswith(b"docker: container ID file found"):
        return True
    return False


def get_file_ignore_ext(directory, file_start):
    for file_name in os.listdir(directory):
        if file_name.split('.')[0] == file_start:
            break
    if not file_name.startswith(file_start):
        print(f'ID {id}: {file_start} file not present')
        return
    return file_name


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


def results_df(base_report, eval_report, ignore_timeout=True):
    
    data = {
        'id': [],
        'model': [],
        'context': [],
        'prompt': [],
        'testcase': [],
        'unittest': [],
        'secure-pass@1': [],
        'sec code block link': [],
        'vul code block link': [],
        'LM code block link': [],
        'description link': [],
        'BM25 retrival link': [],
        'edit distance sec_code_block vs LM_code_block': []
    }

    for id in eval_report.keys():
        for model in eval_report[id].keys():
            for context in eval_report[id][model].keys():
                for prompt in eval_report[id][model][context].keys():

                    data['id'].append(id)
                    data['model'].append(model)
                    data['context'].append(context)
                    data['prompt'].append(prompt)

                    sec_code_block_file = get_file_ignore_ext(f'/home/cdilgren/project_benchmark/descriptions/{id}', 'sec_code_block')
                    data['sec code block link'].append(f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/{sec_code_block_file}')

                    vul_code_block_file = get_file_ignore_ext(f'/home/cdilgren/project_benchmark/descriptions/{id}', 'vul_code_block')
                    data['vul code block link'].append(f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/{vul_code_block_file}')

                    data['LM code block link'].append(f'https://github.com/surrealyz/project_benchmark/tree/connorBranch/completions/{id}/{model}-filled-code-{context}-{prompt}.txt')

                    data['description link'].append(f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/desc.txt')
                    
                    data['BM25 retrival link'].append(f'https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/cross-file.txt')

                    # get sec code block
                    for file_name in os.listdir(f'/home/cdilgren/project_benchmark/descriptions/{id}'):
                        if file_name.split('.')[0] == 'sec_code_block':
                            break
                    if not file_name.startswith('sec_code_block'):
                        print(f'ID {id}: sec_code_block file not present')
                        return
                    with open(f'/home/cdilgren/project_benchmark/descriptions/{id}/{file_name}', "r") as file:
                        sec_code_block = file.read()
                    sec_code_block_lines = re.split('\n', sec_code_block)
                    sec_code_block_lines_clean = [ln.strip() for ln in sec_code_block_lines]
                    sec_code_block_clean = '\n'.join(sec_code_block_lines_clean)

                    # get lm code block
                    with open(f'/home/cdilgren/project_benchmark/completions/{id}/{model}-filled-code-{context}-{prompt}.txt', 'r') as f:
                        lm_code_block = f.read()
                    lm_code_block_lines = re.split('\n', lm_code_block)
                    lm_code_block_lines_clean = [ln.strip() for ln in lm_code_block_lines]
                    lm_code_block_clean = '\n'.join(lm_code_block_lines_clean)

                    dist = distance(sec_code_block_clean, lm_code_block_clean)
                    data['edit distance sec_code_block vs LM_code_block'].append(dist)

                    # If test timed out, then record that entry as Timeout
                    if ignore_timeout:
                        testcase_timeout = get_timeout(f'/home/cdilgren/project_benchmark/oss-fuzz-bench/output/{id}/{model}_{context}_{prompt}_testcase/stderr.txt')
                        if testcase_timeout:
                            data['testcase'].append('Timeout')
                        else:
                            if eval_report[id][model][context][prompt]['testcase'] == 'pass':
                                data['testcase'].append('pass')
                            else:
                                data['testcase'].append('fail')

                        unittest_timeout = get_timeout(f'/home/cdilgren/project_benchmark/oss-fuzz-bench/output/{id}/{model}_{context}_{prompt}_unittest/stderr.txt')
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
    df = results_df(base_report, eval_report, ignore_timeout=True)
    raw_eval_results_path = Path(save_path) / "raw_eval_results.csv"
    df.to_csv(raw_eval_results_path)

    # get results per model, context, and prompt combination
    grouped_results, num_timeouts = analyze_security_results(df)
    grouped_results_path = Path(save_path) / "grouped_results.csv"
    grouped_results.to_csv(grouped_results_path)
    print(f'number of timeouts: {num_timeouts}')


if __name__ == '__main__':

    analyze_report(base_report_path="/home/cdilgren/project_benchmark/oss-fuzz-bench/output/report_unittest_sec.json",
                   eval_report_path="/home/cdilgren/project_benchmark/oss-fuzz-bench/output/report.json",
                   save_path="eval_results",
                   cases_path="/home/cdilgren/project_benchmark/filter_logs/cases.json")
