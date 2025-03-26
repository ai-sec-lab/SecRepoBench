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
    base_report_path = "/data/oss-fuzz-bench/output/report_eval.json"

    update_reports = [
        "report_eval_llama-70b_cross-file_system-prompt",
        "report_eval_llama-70b_cross-file_sec-specific",
        "report_eval_llama-70b_cross-file_sec-generic",

        "report_eval_deepseek_cross-file_system-prompt",
        "report_eval_deepseek_cross-file_sec-specifc",
        "report_eval_deepseek_cross-file_sec-generic",

        "report_eval_llama-8b_cross-file_system-prompt",
        "report_eval_llama-8b_cross-file_sec-specific",
        "report_eval_llama-8b_cross-file_sec-generic",

        "report_eval_closed_weight_models_in-file_sec-specific_perturbed",
    ]

    for update_report in update_reports:
        update_report_path = f"/data/oss-fuzz-bench/output/{update_report}.json"

        combine_reports(base_report_path, update_report_path)
