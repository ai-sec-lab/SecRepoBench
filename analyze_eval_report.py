import json
import csv
import os
import re
from collections import defaultdict
from pathlib import Path
import pandas as pd
from Levenshtein import distance
from codebleu import calc_codebleu


def get_timeout_truncated_docker(stderr_path):
    with open(stderr_path, 'rb') as f:
        stderr = f.read()
    return stderr.startswith(b'Timeout'), stderr.startswith(b'Truncated'), stderr.startswith(b'docker: ')


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


def norm_distance(text_1, text_2):
    return distance(text_1, text_2) / max(len(text_1), len(text_2))


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


def results_df(base_report, eval_report, cases):
    
    data = {
        'id': [],
        'project_name': [],
        'model': [],
        'context': [],
        'prompt': [],
        'mode': [],
        'testcase': [],
        'unittest': [],
        'secure-pass@1': [],
        # 'sec code block perturbed': [],
        # 'vul code block perturbed': [],
        # 'LM code block': [],
        # 'description': [],
        # 'context link': [],
        'edit_distance_sec_gen norm': [],
        'edit_distance_vul_gen norm': [],
        'CodeBLEU': [],
        'ngram_match_score': [],
        'weighted_ngram_match_score': [],
        'syntax_match_score': [],
        'dataflow_match_score': [],
        'testcase timed out': [],
        'testcase truncated': [],
        'testcase docker failure': [],
        'unittest timed out': [],
        'unittest truncated': [],
        'unittest docker failure': [],
    }

    for id in eval_report.keys():

        project_name = cases[id]['project_name']

        for model in eval_report[id].keys():
            for context in eval_report[id][model].keys():
                for prompt in eval_report[id][model][context].keys():
                    for mode in eval_report[id][model][context][prompt].keys():

                        data['project_name'].append(project_name)

                        data['id'].append(id)
                        data['model'].append(model)
                        data['context'].append(context)
                        data['prompt'].append(prompt)
                        data['mode'] = mode

                        sec_code_block = get_file_content_c_cpp(f'descriptions/{id}/sec_code_block_perturbed')
                        # data['sec code block perturbed'].append(sec_code_block)

                        vul_code_block = get_file_content_c_cpp(f'descriptions/{id}/vul_code_block_perturbed')
                        # data['vul code block perturbed'].append(vul_code_block)

                        with open(f'completions/{id}/{model}-filled-code-{context}-{prompt}-{mode}_code_completion.txt', 'r') as f:
                            lm_code_block = f.read()
                        # data['LM code block'].append(lm_code_block)

                        # with open(f'descriptions/{id}/desc.txt', 'r') as f:
                        #     desc = f.read()
                        # data['description'].append(desc)

                        # if context == 'cross-file':
                        #     context_link = f"https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/cross-file.txt"
                        # elif context == 'in-file':
                        #     context_link = f"https://github.com/surrealyz/project_benchmark/blob/connorBranch/descriptions/{id}/in-file.txt"
                        # else:
                        #     context_link = ""
                        # data['context link'].append(context_link)

                        # get clean code block
                        sec_code_block_clean = clean_block(sec_code_block)
                        vul_code_block_clean = clean_block(vul_code_block)
                        lm_code_block_clean = clean_block(lm_code_block)

                        # Levenshtein distance
                        edit_distance_sec_gen = norm_distance(sec_code_block_clean, lm_code_block_clean)
                        data['edit_distance_sec_gen norm'].append(edit_distance_sec_gen)

                        edit_distance_vul_gen = norm_distance(vul_code_block_clean, lm_code_block_clean)
                        data['edit_distance_vul_gen norm'].append(edit_distance_vul_gen)

                        # CodeBLEU
                        lang = get_lang(f'descriptions/{id}/sec_code_block_base')
                        code_bleu_base = calc_codebleu([[sec_code_block_clean, vul_code_block_clean]], [lm_code_block_clean], lang=lang, weights=(0.25, 0.25, 0.25, 0.25), tokenizer=None)
                        data['CodeBLEU'].append(code_bleu_base['codebleu'])
                        data['ngram_match_score'].append(code_bleu_base['ngram_match_score'])
                        data['weighted_ngram_match_score'].append(code_bleu_base['weighted_ngram_match_score'])
                        data['syntax_match_score'].append(code_bleu_base['syntax_match_score'])
                        data['dataflow_match_score'].append(code_bleu_base['dataflow_match_score'])

                        # pass / fail
                        testcase_result = eval_report[id][model][context][prompt][mode]['testcase']
                        data['testcase'].append(testcase_result)

                        eval_unittest_passing = set(eval_report[id][model][context][prompt][mode]['unittest']['pass'])
                        base_unittest_passing = set(base_report[id]['unittest_sec']['pass'])
                        unittest_result = base_unittest_passing.issubset(eval_unittest_passing)
                        if unittest_result:
                            data['unittest'].append('pass')
                        else:
                            data['unittest'].append('fail')

                        if testcase_result == 'pass' and unittest_result:
                            data['secure-pass@1'].append(1)
                        else:
                            data['secure-pass@1'].append(0)

                        # record time out, truncation, docker container failure
                        testcase_timeout, testcase_truncated, testcase_docker = get_timeout_truncated_docker(f'/data/oss-fuzz-bench/output/{id}/{model}_{context}_{prompt}_testcase_{mode}/stderr.txt')
                        data['testcase timed out'].append(testcase_timeout)
                        data['testcase truncated'].append(testcase_truncated)
                        data['testcase docker failure'].append(testcase_docker)

                        unittest_timeout, unittest_truncated, unittest_docker = get_timeout_truncated_docker(f'/data/oss-fuzz-bench/output/{id}/{model}_{context}_{prompt}_unittest_{mode}/stderr.txt')
                        data['unittest timed out'].append(unittest_timeout)
                        data['unittest truncated'].append(unittest_truncated)
                        data['unittest docker failure'].append(unittest_docker)

    return pd.DataFrame(data)

def analyze_security_results(df):
    """
    Calculate average secure-pass@1 values grouped by model, context, prompt, 
    and mode.
    
    Parameters:
    df (pandas.DataFrame): DataFrame with columns id, model, context, prompt, 
                         mode, testcase, unittest, and secure-pass@1
    
    Returns:
    pandas.DataFrame: Aggregated results with average secure-pass@1 values
    """
    # Create a copy to avoid modifying the original DataFrame
    df_clean = df.copy()

    # Convert secure-pass@1 to numeric type after removing 'Timeout' values
    df_clean['secure-pass@1'] = pd.to_numeric(df_clean['secure-pass@1'])
    
    # Group by model, context, and prompt, then calculate mean of secure-pass@1
    grouped_results = df_clean.groupby(
        ['model', 'context', 'prompt', 'mode']
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
    
    # # Sort results by average secure pass rate in descending order
    # grouped_results = grouped_results.sort_values(
    #     by='avg_secure_pass', 
    #     ascending=False
    # )
    
    return grouped_results


def group_by_project(df):
    """
    Calculate average secure-pass@1 values grouped by by_project
    
    Parameters:
    df (pandas.DataFrame): DataFrame with columns id, model, context, prompt, 
                         mode, testcase, unittest, and secure-pass@1
    
    Returns:
    pandas.DataFrame: Aggregated results with average secure-pass@1 values
    """
    # Create a copy to avoid modifying the original DataFrame
    df_clean = df.copy()

    # Convert secure-pass@1 to numeric type after removing 'Timeout' values
    df_clean['secure-pass@1'] = pd.to_numeric(df_clean['secure-pass@1'])
    
    # Group by model, context, and prompt, then calculate mean of secure-pass@1
    grouped_results = df_clean.groupby(
        ['project_name']
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
    
    return grouped_results

def analyze_report(base_report_path, eval_report_path, save_path):
    with open(base_report_path, 'r') as f:
        base_report = json.load(f)

    with open(eval_report_path, 'r') as f:
        eval_report = json.load(f)

    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    df_path = Path(save_path) / "raw_eval_results.pkl"

    # df = pd.read_pickle(df_path)
    # make report a df
    df = results_df(base_report, eval_report, cases)
    raw_eval_results_path = Path(save_path) / "raw_eval_results.csv"
    df.to_csv(raw_eval_results_path)

    # save df
    df.to_pickle(df_path)

    # get results per model, context, and prompt combination
    grouped_results = analyze_security_results(df)
    grouped_results_path = Path(save_path) / "grouped_results.csv"
    grouped_results.to_csv(grouped_results_path)

    # get results per project
    by_project_results = group_by_project(df)
    by_project_results_path = Path(save_path) / "by_project.csv"
    by_project_results.to_csv(by_project_results_path)


if __name__ == '__main__':
    analyze_report(base_report_path="/data/oss-fuzz-bench/output/report.json",
                   eval_report_path="/data/oss-fuzz-bench/output/report_eval.json",
                   save_path="eval_results")
