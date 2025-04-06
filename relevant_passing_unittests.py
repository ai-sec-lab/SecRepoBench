import json
from collections import defaultdict

with open('ids.txt', 'r') as f:
    ids = f.read().splitlines()[1:]

with open('relevant_unittests.json', 'r') as f:
    relevant_unittests = json.load(f)

with open('/data/oss-fuzz-bench/output/report.json', 'r') as f:
    report = json.load(f)

with open('/data/oss-fuzz-bench/output/report_eval.json', 'r') as f:
    eval_report = json.load(f)

with open('filter_logs/cases.json', 'r') as f:
    cases = json.load(f)

# no_relevant_passing = defaultdict(list)
# relevant_passing_unittest = {}
# for id in ids:
#     relevant = relevant_unittests[id]['relevant_unittests']
#     passing = report[id]['unittest_sec']['pass']
#     relevant_passing_unittest[id] = list(set(relevant).intersection(set(passing)))
#     if len(relevant_passing_unittest[id]) == 0:
#         project_name = cases[id]['project_name']
#         no_relevant_passing[project_name].append(id)
# with open('no_relevant_passing.json', 'w') as f:
#     json.dump(no_relevant_passing, f, indent=4)

# missing_names = defaultdict(list)
# for id in ids:
#     # relevant = relevant_unittests[id]['relevant_unittests']
#     passing = report[id]['unittest_sec']['pass']
#     # relevant_passing_unittest[id] = list(set(relevant).intersection(set(passing)))
#     if len(passing) == 0:
#         project_name = cases[id]['project_name']
#         missing_names[project_name].append(id)
# with open('missing_names.json', 'w') as f:
#     json.dump(missing_names, f, indent=4)

# with open('relevant_passing_unittest.json', 'w') as f:
#     json.dump(relevant_passing_unittest, f, indent=4)

id = '17069'
snapshot = report[id]
snapshot['relevant_unittests'] = relevant_unittests[id]['relevant_unittests']
snapshot['eval'] = eval_report[id]
with open('snapshot.json', 'w') as f:
    json.dump(snapshot, f, indent=4)
