import argparse
from tools.patcher import APIEvaler, ChatEvaler, AgentEvaler, ClaudeCodeEvaler
from tqdm import tqdm
from dotenv import load_dotenv
from pathlib import Path
from alive_progress import alive_bar
import sys
import subprocess
from assets.constants import *
from concurrent.futures import ProcessPoolExecutor, as_completed

load_dotenv()


def get_docker_image(id):
    # only pull docker image if we don't already have it
    image_name = f"n132/arvo:{id}-fix"

    proc_check = subprocess.run(
        ["docker", "image", "inspect", image_name],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    if proc_check.returncode != 0:
        # Image does not exist locally, pull it
        proc_pull = subprocess.run(
            ["docker", "pull", image_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        if proc_pull.returncode != 0:
            return False

    return True


def process_id(id, agent, model_name, context_type, prompt_type, mode, rerun):
    if agent == "claudecode":
        evaler = ClaudeCodeEvaler(model_name, context_type, prompt_type, mode)
    elif agent != "none":
        evaler = AgentEvaler(
            agent, model_name, context_type, prompt_type, mode)
    else:
        if 'gpt-' in model_name or 'claude-' in model_name or 'gemini-' in model_name or 'qwen-' in model_name or model_name in API_MODEL_NAMES:
            evaler = APIEvaler(model_name, context_type, prompt_type, mode)
        else:
            evaler = ChatEvaler(model_name, context_type, prompt_type, mode)

    save_path = Path('completions') / id
    save_path.mkdir(parents=True, exist_ok=True)

    response, prompt, system_prompt = evaler.get_response(id, mode, rerun)

    if agent != "none":
        if agent == "claudecode":
            with open(save_path / f'{agent}-{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_code_completion.txt', 'w') as file:
                file.write(response)
        with open(save_path / f'{agent}-{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_prompt.txt', 'w') as file:
            file.write(prompt)
        with open(save_path / f'{agent}-{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_system_prompt.txt', 'w') as file:
            file.write(system_prompt)
    else:
        with open(save_path / f'{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_code_completion.txt', 'w') as file:
            file.write(response)
        with open(save_path / f'{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_prompt.txt', 'w') as file:
            file.write(prompt)
        with open(save_path / f'{model_name}-filled-code-{context_type}-{prompt_type}-{mode}_system_prompt.txt', 'w') as file:
            file.write(system_prompt)
    evaler.save_cache()


def run_inference(agent, model_name, context_type, prompt_type, mode, rerun, max_workers):
    with open('assets/ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    if not rerun:
        print('Using cache where possible')

    if agent != "claudecode" and agent != "none":
        # get any necessary docker images
        print("Downloading any missing docker images")
        with ProcessPoolExecutor(max_workers=max_workers) as executor:
            future_to_stem = {
                executor.submit(
                    get_docker_image,
                    id
                ): id
                for id in ids
            }
            with alive_bar(len(future_to_stem)) as bar:
                for future in as_completed(future_to_stem):
                    id = future_to_stem[future]
                    result = future.result()
                    if result is not True:
                        print(
                            f'Could not download docker image n132/arvo:{id}-fix')
                    bar()

    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = {
            executor.submit(process_id, id, agent, model_name, context_type, prompt_type, mode, rerun): id
            for id in ids
        }

        for future in tqdm(as_completed(futures), total=len(ids), miniters=1, mininterval=0, desc="Processing"):
            try:
                result = future.result()
            except Exception as e:
                print(f'Error processing {futures[future]}: {e}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--agents", nargs='+',
                        help="List of agents to evaluate")
    parser.add_argument('--model-names', nargs='+', type=str)
    parser.add_argument('--context-types', nargs='+', type=str)
    parser.add_argument('--prompt-types', nargs='+', type=str)
    parser.add_argument('--rerun', action="store_true",
                        help="With the rerun flag, it will run inference for a task even if it is in the cache. Otherwise, it will not.")
    args = parser.parse_args()

    max_workers = 5  # Adjust based on API limits
    for agent in args.agents:
        for model_name in args.model_names:
            for context_type in args.context_types:
                for prompt_type in args.prompt_types:
                    for mode in ['perturbed']:
                        print(
                            f"Running inference for {agent}, {model_name}, {context_type}, {prompt_type}, {mode}")

                        run_inference(agent, model_name, context_type,
                                      prompt_type, mode, args.rerun, max_workers)


if __name__ == "__main__":
    main()
