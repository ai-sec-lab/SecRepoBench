from collections import defaultdict
import json


# project[step_lost[id]]
losses = defaultdict(lambda: defaultdict(list))


# get 125
with open('ids_125.txt', 'r') as f:
    ids_125 = f.read().splitlines()[1:]


# get cases, for proj info
with open('/home/cdilgren/project_benchmark/filter_logs/cases.json', 'r') as f:
    cases = json.load(f)


# get ids_each_step.json
with open('/home/cdilgren/project_benchmark/analyze_report/ids_each_step.json', 'r') as f:
    ids_each_step = json.load(f)

# get duplicated ids
with open('duplicate_commits.json', 'r') as f:
    duplicate_commits = json.load(f)

old2new_id = {}
for hash, id_dict in duplicate_commits.items():
    old2new_id[id_dict['old_id']] = id_dict['new_id']

ids_lost = []

for id in ids_125:

    if id not in cases:
        id = old2new_id[id]

    project = cases[id]['project_name']
    if id not in ids_each_step['ids_pass_testcase_sec']:
        losses[project]['not in ids_pass_testcase_sec'].append(id)
        ids_lost.append(id)
    elif id not in ids_each_step['ids_crash_testcase_vul']:
        losses[project]['not in ids_crash_testcase_vul'].append(id)
        ids_lost.append(id)
    elif id not in ids_each_step['ids_pass_unittest_sec']:
        losses[project]['not in ids_pass_unittest_sec'].append(id)
        ids_lost.append(id)
    elif id not in ids_each_step['ids_pass_unittest_sec_print']:
        losses[project]['not in ids_pass_unittest_sec_print'].append(id)
        ids_lost.append(id)
    else:
        losses[project]['pass'].append(id)

with open('losses_125.json', 'w') as f:
    json.dump(losses, f, indent=4)

with open('old2new_dup_id.json', 'w') as f:
    json.dump(old2new_id, f, indent=4)

with open('ids_125_lost.txt', 'w') as f:
    f.write('id\n')
    for id in ids_lost:
        f.write(f'{id}\n')
