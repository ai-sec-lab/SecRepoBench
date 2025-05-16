import os
import json
import argparse
from evaler import APIEvaler, ChatEvaler
from tqdm import tqdm
from dotenv import load_dotenv

load_dotenv()

def main(args):
<<<<<<< HEAD
    if 'gpt-' in args.model_name or 'claude-' in args.model_name or 'gemini-' in args.model_name or 'qwen-' in args.model_name:
=======
    
    if 'gpt-' in args.model_name or 'claude-' in args.model_name or 'gemini-' in args.model_name or 'qwen-' in args.model_name or "llama4" in args.model_name or "llama-maverick" in args.model_name or "Qwen3" in args.model_name:
>>>>>>> 6b1e84907de5435ac2befc1846e1f4c4e1ffc457
        evaler = APIEvaler(args.model_name, args.context_type, args.prompt_type, args.mode)
    else:
        evaler = ChatEvaler(args.model_name, args.context_type, args.prompt_type, args.mode)

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    for id in tqdm(ids):
        # print(id)
        if not os.path.exists(f'completions/{id}'):
            os.makedirs(f'completions/{id}')

        response, prompt, system_prompt = evaler.get_response(id, args.mode)
        with open(f'completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_code_completion.txt', 'w') as file:
            file.write(response)
        with open(f'completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_prompt.txt', 'w') as file:
            file.write(prompt)
        with open(f'completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_system_prompt.txt', 'w') as file:
            file.write(system_prompt)
        evaler.save_cache()
    evaler.save_cache()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model_name', type=str, default='gpt-o1')
    parser.add_argument('--context_type', type=str, default='cross-file')
    parser.add_argument('--prompt_type', type=str, default='system-prompt')
    parser.add_argument('--mode', type=str, default='perturbed')
    args = parser.parse_args()
    main(args)
