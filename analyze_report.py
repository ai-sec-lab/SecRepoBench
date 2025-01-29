import json
import csv
from collections import defaultdict
from pathlib import Path
import pandas as pd


def testcase_sec(report):
    ids_pass_testcase_sec = []
    for id, results in report.items():
        if results.get('testcase_sec', -1) == "pass":
            ids_pass_testcase_sec.append(id)
    return ids_pass_testcase_sec


def testcase_vul(report):
    ids_crash_testcase_vul = []
    for id, results in report.items():
        if results.get('testcase_vul', -1) == "crash":
            ids_crash_testcase_vul.append(id)
    return ids_crash_testcase_vul


def unittest_sec(report):
    ids_pass_unittest_sec = []
    for id, results in report.items():
        if 'unittest_sec' in results and len(results['unittest_sec']['pass']) > 0:
            ids_pass_unittest_sec.append(id)
    return ids_pass_unittest_sec


def unittest_sec_print(report):
    ids_pass_unittest_sec_print = []
    for id in report.keys():
        stdout_path = Path(f'/home/cdilgren/project_benchmark/oss-fuzz-bench/output/{id}/unittest_sec_print/stdout.txt')
        if stdout_path.exists():
            with open(stdout_path, "r", encoding="utf-8", errors="ignore") as f:
                out_text = f.read()
            if 'This is a test for CodeGuard+' in out_text:
                ids_pass_unittest_sec_print.append(id)
    return ids_pass_unittest_sec_print


def both(ids_pass_testcase_sec, ids_crash_testcase_vul):
    ids_pass_testcase = [id for id in ids_pass_testcase_sec if id in ids_crash_testcase_vul]
    return ids_pass_testcase


def make_ids_by_step(
        save_path,
        ids_pass_testcase_sec,
        ids_crash_testcase_vul,
        ids_pass_unittest_sec,
        ids_pass_unittest_sec_print,
        ids_pass_testcase,
        ids_pass_testcase_unittest_sec,
        ids_pass_testcase_unittest):
    ids_each_step = {'ids_pass_testcase_sec': ids_pass_testcase_sec,
                     'ids_crash_testcase_vul': ids_crash_testcase_vul,
                     'ids_pass_unittest_sec': ids_pass_unittest_sec,
                     'ids_pass_unittest_sec_print': ids_pass_unittest_sec_print,
                     'ids_pass_testcase': ids_pass_testcase,
                     'ids_pass_testcase_unittest_sec': ids_pass_testcase_unittest_sec,
                     'ids_pass_testcase_unittest': ids_pass_testcase_unittest}
    ids_each_step_path = save_path / "ids_each_step.json"
    with open(ids_each_step_path, 'w') as f:
        json.dump(ids_each_step, f, indent=4)

    return ids_each_step


def make_ids_by_proj(
        save_path,
        cases_path,
        ids_each_step
    ):
    with open(cases_path, 'r') as f:
        cases = json.load(f)

    ids_each_step_by_proj = {}
    for step, ids in ids_each_step.items():
        proj_ids = defaultdict(list)
        for id in ids:
            proj = cases[id]['project_name']
            proj_ids[proj].append(id)
        ids_each_step_by_proj[step] = proj_ids

    ids_each_step_by_proj_path = save_path / "ids_each_step_by_proj.json"
    with open(ids_each_step_by_proj_path, 'w') as f:
        json.dump(ids_each_step_by_proj, f, indent=4)

    return ids_each_step_by_proj


def add_row_to_df(df, ids_each_step_by_proj, step):
    ids_by_proj = ids_each_step_by_proj[step]
    ids_by_proj_counter = defaultdict(int)
    for column in df.columns:
        if column in ids_by_proj:
            ids_by_proj_counter[column] = len(ids_by_proj[column])
        else:
            ids_by_proj_counter[column] = 0
    row = pd.DataFrame.from_dict([ids_by_proj_counter])
    row.index = [step]
    df = pd.concat([df, row], ignore_index=False)
    return df


def save_to_df(ids_each_step_by_proj, existing_df_path, save_path):
    df = pd.read_csv(existing_df_path, index_col=0)

    # insert pass testcase row
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_pass_testcase')

    # insert pass testcase and unittest sec row
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_pass_testcase_unittest_sec')

    # insert pass testcase, unittest sec, and unittest sec_print row
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_pass_testcase_unittest')

    # save df
    df_path = save_path / "proj_sample_distribution.json"
    df.to_csv(df_path)

    # make and save separate df for singles
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_pass_testcase_sec')
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_crash_testcase_vul')
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_pass_unittest_sec')
    df = add_row_to_df(df, ids_each_step_by_proj, 'ids_pass_unittest_sec_print')
    df_singles = df.tail(4)
    df_singles_path = save_path / "proj_sample_distribution_singles.json"
    df_singles.to_csv(df_singles_path)


def analyze_report(report_path, save_path, cases_path, existing_df_path):
    with open(report_path, 'r') as f:
        report = json.load(f)

    save_path = Path(save_path)

    # get ids passing for sec testcase
    ids_pass_testcase_sec = testcase_sec(report)

    # get ids crash for vul testcase
    ids_crash_testcase_vul = testcase_vul(report)

    # get ids that have at least one passing unittest for sec
    ids_pass_unittest_sec = unittest_sec(report)

    # get ids that have at least one relevant unittest for sec_print
    ids_pass_unittest_sec_print = unittest_sec_print(report)

    # get ids passing for sec testcase and crash for vul testcase
    ids_pass_testcase = both(ids_pass_testcase_sec, ids_crash_testcase_vul)

    # get ids passing testcase and unittest sec
    ids_pass_testcase_unittest_sec = both(ids_pass_testcase, ids_pass_unittest_sec)

    # get ids passing testcase, unittest sec, and unittest sec_print
    ids_pass_testcase_unittest = both(ids_pass_testcase_unittest_sec, ids_pass_unittest_sec_print)

    # save ids to json
    ids_each_step = make_ids_by_step(
        save_path,
        ids_pass_testcase_sec,
        ids_crash_testcase_vul,
        ids_pass_unittest_sec,
        ids_pass_unittest_sec_print,
        ids_pass_testcase,
        ids_pass_testcase_unittest_sec,
        ids_pass_testcase_unittest)

    # save ids by project to json
    ids_each_step_by_proj = make_ids_by_proj(
        save_path,
        cases_path,
        ids_each_step
    )

    # save to dataframe
    save_to_df(ids_each_step_by_proj, existing_df_path, save_path)


if __name__ == '__main__':

    analyze_report(report_path="/home/cdilgren/project_benchmark/oss-fuzz-bench/output/report.json",
                   save_path="analyze_report",
                   cases_path="/home/cdilgren/project_benchmark/filter_logs/cases.json",
                   existing_df_path="/home/cdilgren/project_benchmark/filter_logs/proj_samples.csv")
