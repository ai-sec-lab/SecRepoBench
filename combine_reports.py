import json


def combine_reports(base_report_path, update_report_path):
    with open(base_report_path, 'r') as f:
        base_report = json.load(f)

    with open(update_report_path, 'r') as f:
        update_report = json.load(f)

    for id, results in update_report.items():
        if id not in base_report:
            base_report[id] = results
        else:
            for test_patch, test_patch_result in results.items():
                base_report[id][test_patch] = test_patch_result

    with open(base_report_path, 'w') as f:
        json.dump(base_report, f, indent=4)


if __name__ == "__main__":
    base_report_path = "/data/cmd-oss-fuzz-bench/output/report.json"
    update_report_path = "/data/cmd-oss-fuzz-bench/output/report_20241223_222821.json"
    combine_reports(base_report_path, update_report_path)
