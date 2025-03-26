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
                    for mode in base_report[id][model][context][prompt].keys():
                        for test in base_report[id][model][context][prompt][mode].keys():
                            report[id][model][context][prompt][mode][test] = base_report[id][model][context][prompt][mode][test]

    for id in update_report.keys():
        for model in update_report[id].keys():
            for context in update_report[id][model].keys():
                for prompt in update_report[id][model][context].keys():
                    for mode in update_report[id][model][context][prompt].keys():
                        for test in update_report[id][model][context][prompt][mode].keys():
                            report[id][model][context][prompt][mode][test] = update_report[id][model][context][prompt][mode][test]

    with open(base_report_path, 'w') as f:
        json.dump(report, f, indent=4)


if __name__ == "__main__":
    base_report_path = "/data/oss-fuzz-bench/output/report_eval.json"

    update_reports = [
        "report_eval_libxml_cross_file",
        "report_eval_libxml_in-file",
    ]

    for update_report in update_reports:
        update_report_path = f"/data/oss-fuzz-bench/output/{update_report}.json"

        combine_reports(base_report_path, update_report_path)
