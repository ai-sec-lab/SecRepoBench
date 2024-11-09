import json
import os
import csv
import argparse

def get_id_with_right_patches():
    with open('/data/fyj-oss-fuzz-bench/output/report.json', 'r') as f:
        report = json.load(f)
    print(len(report))

    reliable_ids = []
    wrong_ids = []
    double_check_ids = []

    for id in report:
        if report[id]['sec']['testcase'] == 'crash' or report[id]['vul']['testcase'] == 'pass':
            wrong_ids.append(id)
        elif report[id]['sec']['testcase'] == 'pass' and report[id]['vul']['testcase'] == 'crash':
            reliable_ids.append(id)
        else:
            double_check_ids.append(id)

    print(f'wrong_ids: {len(wrong_ids)}')
    print(f'double_check_ids: {len(double_check_ids)}')
    print(f'reliable_ids: {len(reliable_ids)}')

    return reliable_ids

def get_ids_with_right_unittest():
    with open('/data/fyj-oss-fuzz-bench/output/report.json', 'r') as f:
        report = json.load(f)
    ids = get_id_with_right_patches()
    print(len(ids))
    testable_ids = []
    for id in ids:
        case = report[id]
        set_vul_pass = set([res.strip() for res in case["vul"]["unittest"]["pass"]])
        set_sec_pass = set([res.strip() for res in case["sec"]["unittest"]["pass"]])
        if len(set_vul_pass) >0 and set_vul_pass <= set_sec_pass:
            testable_ids.append(id)

    print(f'testable_ids: {len(testable_ids)}')
    with open('ids/testable_ids.txt', 'w') as f:
        for id in testable_ids:
            f.write(f'{id}\n')
    # write to csv, the first row is the header 'ID'
    with open('ids/testable_ids.csv', 'w') as f:
        writer = csv.writer(f)
        writer.writerow(['ID'])
        for id in testable_ids:
            writer.writerow([id])

def get_ids_pass_print_check():
    with open('ids/testable_ids.txt', 'r') as f:
        ids = f.readlines()
    ids = [id.strip() for id in ids]
    print(len(ids))
    valid_ids = []
    for id in ids:
        with open(f'/data/fyj-oss-fuzz-bench/output/{id}/vul_print/unittest/stdout.txt', "r", encoding="utf-8", errors="ignore") as f:
            out_text = f.read()
        if 'This is a test for CodeGuard+' in out_text:
            valid_ids.append(id)

    print(f'valid_ids: {len(valid_ids)}')
    with open('ids/print_checked_ids.txt', 'w') as f:
        for id in valid_ids:
            f.write(f'{id}\n')
    # write to csv, the first row is the header 'ID'
    with open('ids/print_checked_ids.csv', 'w') as f:
        writer = csv.writer(f)
        writer.writerow(['ID'])
        for id in valid_ids:
            writer.writerow([id])
    print(valid_ids)

def get_covered_ids():
    with open('/data/fyj-oss-fuzz-bench/output/report.json', 'r') as f:
        report = json.load(f)          
    print(len(report))

    covered_ids = []
    suspect_ids = []
    for id in report:
        print(id)
        case = report[id]
        set_vul_pass = set([res.strip() for res in case["vul"]["unittest"]["pass"]])
        set_mask_pass = set([res.strip() for res in case["mask"]["unittest"]["pass"]])
        set_sec_pass = set([res.strip() for res in case["sec"]["unittest"]["pass"]])
        if set_vul_pass != set_mask_pass and \
            not (set_vul_pass <= set_mask_pass) and \
            len(set_sec_pass) > 0 and len(set_vul_pass) > 0:
            if set_vul_pass <= set_sec_pass:
                    covered_ids.append(id)
            else:
                suspect_ids.append(id)


    print(f'covered_ids: {len(covered_ids)}')
    with open('ids/covered_ids.txt', 'w') as f:
        for id in covered_ids:
            f.write(f'{id}\n')
    # write to csv, the first row is the header 'ID'
    with open('ids/covered_ids.csv', 'w') as f:
        writer = csv.writer(f)
        writer.writerow(['ID'])
        for id in covered_ids:
            writer.writerow([id])

if __name__ == '__main__':
    argparse = argparse.ArgumentParser()
    argparse.add_argument('--step', type=str, default='coverage')

    args = argparse.parse_args()
    step = args.step
    if step == 'execution':
        get_ids_with_right_unittest()
    if step == 'print_check':
        get_ids_pass_print_check()
    elif step == 'coverage':
        get_covered_ids()