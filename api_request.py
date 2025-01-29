import os
import json
import argparse
from evaler import APIEvaler, ChatEvaler
from tqdm import tqdm

os.environ["ANTHROPIC_API_KEY"] = ""
os.environ["OPENAI_API_KEY"] = ""
os.environ["GOOGLE_API_KEY"] = ""  

def main(args):
    if 'gpt-' in args.model_name or 'claude-' in args.model_name or 'gemini-' in args.model_name:
        evaler = APIEvaler(args.model_name, args.context_type, args.prompt_type)
    else:
        evaler = ChatEvaler(args.model_name, args.context_type, args.prompt_type)

    with open('final_ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    ids = sorted([int(id) for id in ids])
    num = 0
    for id in tqdm(ids):
        id = str(id)
        if not os.path.exists(f'completions/{id}'):
            os.makedirs(f'completions/{id}')

        num += 1
        response = evaler.get_response(id)
        with open(f'completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}.txt', 'w') as file:
            file.write(response)
        evaler.save_cache()
    evaler.save_cache()
    print(num)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model_name', type=str, default='claude-3-haiku')
    parser.add_argument('--context_type', type=str, default='cross-file')
    parser.add_argument('--prompt_type', type=str, default='refine')
    args = parser.parse_args()
    main(args)