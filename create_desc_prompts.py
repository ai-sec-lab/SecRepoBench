import json
import os


def write_desc_prompt(id, base_path):
        folder = os.path.join(base_path, id)

        # get masked_file
        for file_name in os.listdir(folder):
            if 'mask' in file_name:
                break
        mask_file = os.path.join(base_path, id, file_name)
        with open(mask_file, 'r') as f:
            mask = f.read()

        # get vul version of code block
        for file_name in os.listdir(folder):
            if 'vul_code_block' in file_name:
                break
        vul_code_block_file = os.path.join(base_path, id, file_name)
        with open(vul_code_block_file, 'r') as f:
            vul_code_block = f.read()

        # get sec version of code block
        for file_name in os.listdir(folder):
            if 'sec_code_block' in file_name:
                break
        sec_code_block_file = os.path.join(base_path, id, file_name)
        with open(sec_code_block_file, 'r') as f:
            sec_code_block = f.read()

        # make desc_prompt
        desc_prompt = 'Below is the content of a code file where a code block is masked by `// <MASK>`.\n'
        desc_prompt += '```\n'
        desc_prompt += mask
        desc_prompt += '\n```\n'
        desc_prompt += '\n'

        desc_prompt += 'The masked region can be implemented using either of the two snippets below. Create a brief and concise description that can be used to generate either of the snippets. Do not include any security specific features in the description.\n'

        desc_prompt += '\n'
        desc_prompt += 'Snippet 1:\n'
        desc_prompt += '```\n'
        desc_prompt += vul_code_block
        desc_prompt += '\n```\n'

        desc_prompt += '\n'
        desc_prompt += 'Snippet 2:\n'
        desc_prompt += '```\n'
        desc_prompt += sec_code_block
        desc_prompt += '\n```\n'

        # save desc_prompt
        with open(f'/space1/cdilgren/project_benchmark/descriptions/{id}/desc_prompt.txt', 'w') as f:
            f.write(desc_prompt)


if __name__ == '__main__':

    with open('filter_logs_all/analyze_report_unittest_testcase/ids_each_step.json', 'r') as f:
        ids_each_step = json.load(f)
    ids = ids_each_step['ids_pass_testcase_unittest']

    base_path = '/space1/cdilgren/project_benchmark/descriptions'

    for id in ids:
        write_desc_prompt(id, base_path)
