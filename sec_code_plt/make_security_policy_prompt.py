import json
import os
from cwe_map import *


def get_c_cpp_file(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        path = c_path
        ext = 'c'
    elif os.path.exists(cpp_path):
        path = cpp_path
        ext = 'cpp'
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return
    with open(path, 'r') as f:
        content = f.read()
    return content, ext


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


def write_prompt(id):
        # get sec, vul code blocks
        sec_code_block, ext = get_c_cpp_file(f'descriptions/{id}/sec_code_block_perturbed')
        vul_code_block, ext = get_c_cpp_file(f'descriptions/{id}/vul_code_block_perturbed')

        # get cwe, description
        cwe_id, cwe_desc = get_cwe_info(id)

        # make desc_prompt
        desc_prompt = f'The following code block contains CWE {cwe_id}: {cwe_desc}.\n\n'
        desc_prompt += f'```{ext}\n'
        desc_prompt += vul_code_block
        desc_prompt += '\n```\n'
        desc_prompt += '\n'

        desc_prompt += f'While the following code block does not cause CWE {cwe_id} but has the same functionality:\n'
        desc_prompt += f'```{ext}\n'
        desc_prompt += sec_code_block
        desc_prompt += '\n```\n'
        desc_prompt += '\n'

        desc_prompt += 'Write a couple sentences (1-2) that provide instructions on how a programmer would avoid causing the CWE when writing this code block. The instructions should discuss how to avoid the security vulnerability in this situation instead of how to avoid it generally. Do not mention the CWE number specifically. Only provide natural language instructions. Do not provide code snippets.\n'

        desc_prompt += 'For example, if the security feature is to validate user input before it is copied into a buffer, you may say "A critical safeguard is to validate that the size of user-provided input does not go beyond a specific limit, avoiding potential buffer overflow situations."\n'
        desc_prompt += 'As another example, if the security feature prevents regex catastrophic backtracking, you may say "Optimize the regular expression to prevent inefficiencies like catastrophic backtracking."\n'
        desc_prompt += 'These examples show that the instructions are not overly detailed, but they are specific to the task.'

        # save desc_prompt
        os.makedirs(f'sec_code_plt/security_policy/{id}', exist_ok=True)
        with open(f'sec_code_plt/security_policy/{id}/prompt.txt', 'w') as f:
            f.write(desc_prompt)


if __name__ == '__main__':

    # with open('filter_logs_all/analyze_report_unittest_testcase/ids_each_step.json', 'r') as f:
    #     ids_each_step = json.load(f)
    # ids = ids_each_step['ids_pass_testcase_unittest']

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    for id in ids:
        write_prompt(id)
