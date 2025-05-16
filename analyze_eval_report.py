import json
from pathlib import Path
import pandas as pd
from cwe_map import *


def get_cwe_info(crash_type):
    if crash_type == 'UNKNOWN WRITE':
        pass
    elif crash_type == 'UNKNOWN READ':
        pass
    elif crash_type == 'Segv on unknown address':
        pass
    else:
        crash_type = crash_type.split()[0]

    cwe_id = crash_type_to_cwe[crash_type]
    return cwe_id


def get_timeout_truncated_docker(stderr_path):
    with open(stderr_path, 'rb') as f:
        stderr = f.read()
    return stderr.startswith(b'Timeout'), stderr.startswith(b'Truncated'), stderr.startswith(b'docker: ')


def results_df(base_report, eval_report, sample_metadata, ids):
    
    data = {
        'id': [],
        'CWE ID': [],
        'project_name': [],
        'model': [],
        'context': [],
        'prompt': [],
        'mode': [],
        'testcase': [],
        'unittest': [],  # note: only the testcase tracks compiler failures. This is fail if it does not compile or does compile and fails unittests
        'secure-pass@1': [],
        'testcase timed out': [],
        'testcase truncated': [],
        'testcase docker failure': [],
        'unittest timed out': [],
        'unittest truncated': [],
        'unittest docker failure': [],
    }

    for id in ids:

        if id not in eval_report.keys():
            print(f'Missing results for id {id}')
            continue

        project_name = sample_metadata[id]['project_name']
        CWE_id = get_cwe_info(sample_metadata[id]['crash_type'])

        for model in eval_report[id].keys():
            for context in eval_report[id][model].keys():
                for prompt in eval_report[id][model][context].keys():
                    for mode in eval_report[id][model][context][prompt].keys():

                        data['project_name'].append(project_name)
                        data['CWE ID'].append(CWE_id)

                        data['id'].append(id)
                        data['model'].append(model)
                        data['context'].append(context)
                        data['prompt'].append(prompt)
                        data['mode'] = mode

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

                        testcase_timeout, testcase_truncated, testcase_docker = get_timeout_truncated_docker(f'data/{id}/{model}_{context}_{prompt}_testcase_{mode}/stderr.txt')
                        data['testcase timed out'].append(testcase_timeout)
                        data['testcase truncated'].append(testcase_truncated)
                        data['testcase docker failure'].append(testcase_docker)

                        unittest_timeout, unittest_truncated, unittest_docker = get_timeout_truncated_docker(f'data/{id}/{model}_{context}_{prompt}_unittest_{mode}/stderr.txt')
                        data['unittest timed out'].append(unittest_timeout)
                        data['unittest truncated'].append(unittest_truncated)
                        data['unittest docker failure'].append(unittest_docker)

    return pd.DataFrame(data)


def grouped_metrics(df):
    # Copy dataframe to avoid mutating original
    df_clean = df.copy()

    group_cols = ['model', 'context', 'prompt', 'mode']

    # --- secure-pass@1 ---
    df_clean['secure-pass@1'] = pd.to_numeric(df_clean['secure-pass@1'], errors='coerce')
    secure_pass = df_clean.groupby(group_cols)['secure-pass@1'].agg([
        ('avg_secure_pass_at_1', 'mean'),
        ('num_samples_secure_pass_at_1', 'count'),
        ('std_dev_secure_pass_at_1', 'std')
    ]).round(4).reset_index()

    # --- pass@1 from unittest ---
    df_clean['pass@1'] = df_clean['unittest'].map({'pass': 1, 'fail': 0})
    pass_at_one = df_clean.groupby(group_cols)['pass@1'].agg([
        ('avg_pass_at_one', 'mean'),
        ('num_samples_pass_at_one', 'count'),
        ('std_dev_pass_at_one', 'std')
    ]).round(4).reset_index()

    # --- percent_not_compiled (not "pass" or "crash" in testcase) ---
    df_clean['not_compiled'] = (~df_clean['testcase'].isin(['pass', 'crash'])).astype(int)
    percent_not_compiled = df_clean.groupby(group_cols)['not_compiled'].agg([
        ('percent_not_compiled', lambda x: round(x.mean(), 2)),
        ('num_not_compiled_samples', 'sum')
    ]).reset_index()
    
    # --- incorrect samples (testcase in ["pass", "crash"] (which means it compiled) and unittest is "fail") ---
    df_clean['incorrect'] = (
        (df_clean['testcase'].isin(['pass', 'crash'])) & 
        (df_clean['unittest'] == 'fail')
    ).astype(int)
    incorrect_samples = df_clean.groupby(group_cols)['incorrect'].agg([
        ('num_incorrect_samples', 'sum')
    ]).reset_index()
    
    # --- secure but incorrect samples (testcase is "pass" and unittest is "fail") ---
    df_clean['secure_but_incorrect'] = (
        (df_clean['testcase'] == 'pass') & 
        (df_clean['unittest'] == 'fail')
    ).astype(int)
    secure_but_incorrect = df_clean.groupby(group_cols)['secure_but_incorrect'].agg([
        ('num_secure_but_incorrect', 'sum')
    ]).reset_index()

    # --- secure samples (testcase is "pass") ---
    df_clean['secure'] = (
        (df_clean['testcase'] == 'pass')
    ).astype(int)
    secure = df_clean.groupby(group_cols)['secure'].agg([
        ('num_secure', 'sum')
    ]).reset_index()


    # --- Combine all results ---
    combined = secure_pass \
        .merge(pass_at_one, on=group_cols, how='outer') \
        .merge(percent_not_compiled, on=group_cols, how='outer') \
        .merge(incorrect_samples, on=group_cols, how='outer') \
        .merge(secure_but_incorrect, on=group_cols, how='outer') \
        .merge(secure, on=group_cols, how='outer' )

    return combined


def analyze_report(ids, eval_report_path):
    # answer key for unittests
    with open('report.json', 'r') as f:
        base_report = json.load(f)

    with open(eval_report_path, 'r') as f:
        eval_report = json.load(f)

    with open('sample_metadata.json', 'r') as f:
        sample_metadata = json.load(f)

    save_path = Path('eval_results')
    save_path.mkdir(parents=True, exist_ok=True)

    df_path = save_path / "raw_eval_results.pkl"

    # all results
    df = results_df(base_report, eval_report, sample_metadata, ids)
    raw_eval_results_path = Path(save_path) / "raw_eval_results.csv"
    df.to_csv(raw_eval_results_path)
    df.to_pickle(df_path)

    # ground results by model, context, and prompt combination
    grouped_results = grouped_metrics(df)
    grouped_results_path = Path(save_path) / "grouped_results.csv"
    grouped_results.to_csv(grouped_results_path)


if __name__ == "__main__":
    # evaluate all ids in ids.txt
    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    eval_report_path = "report_eval_DeepSeek-R1-Distill-Qwen-32B.json"

    analyze_report(ids, eval_report_path)
