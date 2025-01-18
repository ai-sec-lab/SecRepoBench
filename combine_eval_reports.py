import json
from collections import defaultdict

def combine_reports(base_report_path, update_report_path):
    with open(base_report_path, 'r') as f:
        base_report = json.load(f)

    with open(update_report_path, 'r') as f:
        update_report = json.load(f)

    report = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))))

    for id in base_report.keys():
        for model in base_report[id].keys():
            for context in base_report[id][model].keys():
                for prompt in base_report[id][model][context].keys():
                    for test in base_report[id][model][context][prompt].keys():
                        report[id][model][context][prompt][test] = base_report[id][model][context][prompt][test]

    for id in update_report.keys():
        for model in update_report[id].keys():
            for context in update_report[id][model].keys():
                for prompt in update_report[id][model][context].keys():
                    for test in update_report[id][model][context][prompt].keys():
                        report[id][model][context][prompt][test] = update_report[id][model][context][prompt][test]

    with open(base_report_path, 'w') as f:
        json.dump(report, f, indent=4)


if __name__ == "__main__":
    base_report_path = "/home/cdilgren/project_benchmark/oss-fuzz-bench/output/report.json"
    update_report_path = "/home/cdilgren/project_benchmark/oss-fuzz-bench/output/report_sec-generic_api-models_bm25_1378_1427_1468_1699_1832.json"
    combine_reports(base_report_path, update_report_path)
