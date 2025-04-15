import argparse
from evaler import APIEvaler, ChatEvaler
from tqdm import tqdm
from dotenv import load_dotenv
from pathlib import Path

load_dotenv()


def run_inference(model_name, context_type, prompt_type, mode, rerun):
    if 'gpt-' in model_name or 'claude-' in model_name or 'gemini-' in model_name or 'qwen-' in model_name:
        evaler = APIEvaler(model_name, context_type, prompt_type, mode)
    else:
        evaler = ChatEvaler(model_name, context_type, prompt_type, mode)

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    if not rerun:
        print('Using cache where possible')

    for id in tqdm(ids):
        save_path = Path('completions') / id
        save_path.mkdir(parents=True, exist_ok=True)

        response, prompt, system_prompt = evaler.get_response(id, mode, rerun)
        with open(save_path / f'{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_code_completion.txt', 'w') as file:
            file.write(response)
        with open(save_path / f'{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_prompt.txt', 'w') as file:
            file.write(prompt)
        with open(save_path / f'{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_system_prompt.txt', 'w') as file:
            file.write(system_prompt)
        evaler.save_cache()
    evaler.save_cache()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model_names', nargs='+', type=str)
    parser.add_argument('--context_types', nargs='+', type=str)
    parser.add_argument('--prompt_types', nargs='+', type=str)
    parser.add_argument('--rerun', action="store_true", help="With the rerun flag, it will run inference for a task even if it is in the cache. Otherwise, it will not.")
    args = parser.parse_args()

    for model_name in args.model_names:
        for context_type in args.context_types:
            for prompt_type in args.prompt_types:
                for mode in ['perturbed']:
                    print(f"Running inference for {model_name}, {context_type}, {prompt_type}, {mode}")
                    run_inference(model_name, context_type, prompt_type, mode, args.rerun)
