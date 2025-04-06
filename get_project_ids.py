import json


with open('filter_logs/cases.json', 'r') as f:
    cases = json.load(f)

with open('ids.txt', 'r') as f:
    ids = f.read().splitlines()[1:]

project_name = 'libxml2'

proj_ids = []

for id in ids:
    case = cases[id]
    if project_name == case['project_name']:
        proj_ids.append(id)

with open(f'ids_{project_name}.txt', 'w') as f:
    f.write('id\n')
    for id in proj_ids:
        f.write(f'{id}\n')
