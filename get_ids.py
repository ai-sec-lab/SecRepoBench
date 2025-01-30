import json
import os


with open('filter_logs/samples_each_step.json', 'r') as f:
    samples_each_step = json.load(f)
ids_top40 = [str(id) for id in sorted([int(id) for id in samples_each_step[-1]['sample ids present']])]

with open('ids_125.txt', 'r') as f:
    ids_125 = f.read().splitlines()[1:]

ids_new = [id for id in ids_top40 if id not in ids_125]
with open('ids_new.txt', 'w') as f:
    f.write('id\n')
    for id in ids_new:
        f.write(f'{id}\n')
