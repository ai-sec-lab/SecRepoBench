from tools.patcher import APIPatcher, ChatPatcher, AgentPatcher, ClaudeCodePatcher
from tqdm import tqdm
from pathlib import Path
from alive_progress import alive_bar
import subprocess
import os
import json
from assets.constants import *
from concurrent.futures import ProcessPoolExecutor, as_completed
from tools.utils import get_docker_image, docker_image_exists


def process_id(id, agent, model_name, context_type, prompt_type, mode, rerun):
    if agent == "claudecode":
        patcher = ClaudeCodePatcher(model_name, context_type, prompt_type, mode)
    elif agent != "none":
        patcher = AgentPatcher(
            agent, model_name, context_type, prompt_type, mode)
    else:
        if 'gpt-' in model_name or 'claude-' in model_name or 'gemini-' in model_name or 'qwen-' in model_name or model_name in API_MODEL_NAMES:
            patcher = APIPatcher(model_name, context_type, prompt_type, mode)
        else:
            patcher = ChatPatcher(model_name, context_type, prompt_type, mode)

    save_path = Path('completions') / id

    response, prompt, system_prompt = patcher.get_response(id, mode, rerun)

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
    patcher.save_cache()


def inference(ids, agent, model_name, context_type, prompt_type, mode, rerun, max_workers):
    if not rerun:
        print('Using cache where possible')

    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        futures = {
            executor.submit(process_id, id, agent, model_name, context_type, prompt_type, mode, rerun): id
            for id in ids
        }

        for future in tqdm(as_completed(futures), total=len(ids), miniters=1, mininterval=0, desc="Processing"):
            try:
                _ = future.result()
            except Exception as e:
                print(f'Error processing {futures[future]}: {e}')


def run_inferences(ids, agents, model_names, prompt_types, context_types, rerun, modes, num_workers):
    for agent in agents:
        for model_name in model_names:
            for context_type in context_types:
                for prompt_type in prompt_types:
                    for mode in modes:
                        print(
                            f"Running inference for {agent}, {model_name}, {context_type}, {prompt_type}, {mode}")

                        inference(ids, agent, model_name, context_type,
                                  prompt_type, mode, rerun, num_workers)


def init_docker(id, rerun):
    save_path = Path('completions') / id
    save_path.mkdir(parents=True, exist_ok=True)
    container_id = f"{id}"

    if not rerun and docker_image_exists(f"secrepobench:{id}"):
        print(f"Use cache for docker image secrepobench:{id}")
        return True

    base_dir = os.getcwd()
    sample_metadata = json.load(open('sample_metadata.json', "r"))
    project_name = sample_metadata[id]["project_name"]
    changed_file = sample_metadata[id]["changed_file"]
    fixing_commit = sample_metadata[id]["fixing_commit"]

    volumes = [
        f"{base_dir}/descriptions/{id}:/descriptions",
    ]
    volume_flags = " ".join([f"-v {vol}" for vol in volumes])

    content = (
        "#!/bin/bash\n"
        "docker run --init "
        f"--name {container_id} "
        "--cpus=2 "
        f"{volume_flags} "
        f"n132/arvo:{id}-fix /bin/sh -c \"\n"
        # limit num processes to 2 by changing nproc behavior
        "  echo '#!/bin/sh' > /tmp/nproc\n"
        "  echo 'echo 2' >> /tmp/nproc\n"
        "  chmod +x /tmp/nproc\n"
        "  export PATH=/tmp:$PATH\n"
        # revert to fixing commit and stash changes as necessary
        f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
        "  git -C \\$GIT_DIR config --global user.email \\\"anonymous@email.com\\\"\n"
        "  if [ -n \\\"\\$(git -C \\$GIT_DIR status --porcelain)\\\" ]; then\n"
        "    git -C \\$GIT_DIR stash save --include-untracked \\\"Saving my changes\\\"\n"
        "    CHANGES_STASHED=true\n"
        "  else\n"
        "    CHANGES_STASHED=false\n"
        "  fi\n"
        f"  git -C \\$GIT_DIR checkout {fixing_commit}\n"
        "  if [ \\\"\\$CHANGES_STASHED\\\" = true ]; then\n"
        "    git -C \\$GIT_DIR stash apply\n"
        "  fi\n"
        # install global dependencies
        "  mkdir /workdir && cd /workdir\n"
        "  apt install -y curl\n"
        "  curl -LsSf https://astral.sh/uv/install.sh | sh \n"
        "  cd \\$GIT_DIR\n"
        f"  cp /descriptions/mask_desc_perturbed.c {changed_file} 2>/dev/null || cp /descriptions/mask_desc_perturbed.cpp {changed_file} 2>/dev/null || echo 'File not found'\n"
        "  rm -rf .git\n"
        "  git init && git add . && git commit -m 'init commit'\n"
        "\"\n"
        f"docker commit {container_id} secrepobench:{id}\n"
        f"docker rm {container_id}\n"
    )

    bash_dir = save_path / f"agent_docker_init.sh"
    bash_dir.write_text(content)

    try:
        subprocess.run(['/bin/bash', str(bash_dir)],
                       check=True,
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"Error: {e}")
        return False

    return True


def docker_setup(ids, num_workers, rerun):
    # get any necessary docker images
    print("Downloading any missing docker images")
    with ProcessPoolExecutor(max_workers=num_workers) as executor:
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

    print("Initialize docker images")
    if not rerun:
        print('Using cache where possible')

    with ProcessPoolExecutor(max_workers=num_workers) as executor:
        future_to_stem = {
            executor.submit(
                init_docker,
                id, rerun
            ): id
            for id in ids
        }
        with alive_bar(len(future_to_stem)) as bar:
            for future in as_completed(future_to_stem):
                id = future_to_stem[future]
                result = future.result()
                if result is not True:
                    print(
                        f'Could not create docker image secrepobench:{id}')
                bar()
