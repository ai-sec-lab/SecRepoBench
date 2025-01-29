import json
from cwe_map import *


def get_cwe_info(id):
    with open(f'/home/cdilgren/project_benchmark/ARVO-Meta/meta/{id}.json', 'r') as f:
        meta = json.load(f)
    
    crash_type = meta['crash_type']

    if crash_type == 'UNKNOWN WRITE':
        pass
    elif crash_type == 'UNKNOWN READ':
        pass
    elif crash_type == 'Segv on unknown address':
        pass
    else:
        crash_type = crash_type.split()[0]

    cwe_id = crash_type_to_cwe[crash_type]
    cwe_desc = cwe_id_to_desc[cwe_id]
    return cwe_id, cwe_desc


with open('final_ids.txt', 'r') as f:
    ids = f.read().splitlines()[1:]

for id in ids:
    cwe_id, cwe_desc = get_cwe_info(int(id))
