import json


with open('filter_logs/cases.json', 'r') as f:
    cases = json.load(f)

with open('/home/cdilgren/project_benchmark/ids_top40_mask.txt', 'r') as f:
    ids = f.read().splitlines()[1:]


ids_opensc = []
ids_opensc_asn1 = []

for id in ids:
    case = cases[id]
    project = case['project_name']
    if project == 'opensc':
        ids_opensc.append(id)
        changed_file = case['changed_file']
        if changed_file.endswith('asn1.c'):
            ids_opensc_asn1.append(id)

print(ids_opensc)
print(ids_opensc_asn1)

with open('ids_opensc.txt', 'w') as f:
    f.write('id\n')
    for id in ids_opensc:
        f.write(f'{id}\n')
