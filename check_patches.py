import os


with open('final_ids.txt', 'r') as f:
    ids = f.read().splitlines()[1:]

models = ['claude-3-haiku', 'claude-3.5-sonnet', 'gemini-1.5-flash', 'gemini-1.5-pro', 'gpt-4o-mini', 'gpt-4o']
contexts = ['cross-file']
prompts = ['sec-generic', 'sec-specific', 'system-prompt']

for id in ids:
    for model in models:
        for context in contexts:
            for prompt in prompts:
                    file = f'/home/cdilgren/project_benchmark/oss-fuzz-bench/{id}/patches/patch_{model}_filled_code_{context}_{prompt}.txt'
                    if not os.path.isfile(file):
                        print(file)
