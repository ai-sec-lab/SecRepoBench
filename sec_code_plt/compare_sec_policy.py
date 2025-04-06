import json
from cwe_map import *


with open('sec_code_plt/data_one.json', 'r') as f:
    data_one = json.load(f)

cwe_to_sec_policy = {}

for data in data_one:
    cwe = data['CWE_ID']
    sec_policy = data['task_description']['security_policy']

    if cwe not in cwe_to_sec_policy:
        cwe_to_sec_policy[cwe] = sec_policy

with open('sec_code_plt/cwe_to_sec_policy.json', 'w') as f:
    json.dump(cwe_to_sec_policy, f, indent=4)

cwes_sec_code_plt = [int(cwe) for cwe in cwe_to_sec_policy.keys()]
cwes_sec_code_plt.sort()

our_cwes = list(cwe_id_to_desc.keys())
our_cwes.sort()

same = [c for c in our_cwes if c in cwes_sec_code_plt]
new = [c for c in our_cwes if c not in cwes_sec_code_plt]

print(f"same: {same}")
print(f"new: {new}")

def get_cwe_info(id):
    with open(f'ARVO-Meta/meta/{id}.json', 'r') as f:
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

print(get_cwe_info('910'))