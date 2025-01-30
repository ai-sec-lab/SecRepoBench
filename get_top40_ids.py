import json


with open('filter_logs/samples_each_step.json', 'r') as f:
    samples_each_step = json.load(f)
ids = [str(id) for id in sorted([int(id) for id in samples_each_step[-1]['sample ids present']])]

with open('ids_top40.txt', 'w') as f:
    f.write('id\n')
    for id in ids:
        f.write(f'{id}\n')
