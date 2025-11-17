import os
import json
import time
from contextlib import redirect_stdout, redirect_stderr
import shutil
import threading
from pathlib import Path
from git import Repo
import argparse
from openhands.sdk import LLM, Agent, Conversation
from openhands.tools.preset.default import get_default_tools
from constants import *


# Openhands has its own configure file, need to map the exact same model name
MODEL_MAPPINGS = {
    'gpt-4.1-2025-04-14': 'gpt-4.1',
    'o4-mini-2025-04-16': 'o4-mini',
    'o3-2025-04-16': 'o3',
}


def get_c_cpp_file(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        path = c_path
    elif os.path.exists(cpp_path):
        path = cpp_path
    else:
        print(
            f'This file does not exist with a c or cpp extension: {base_path}')
        return
    with open(path, 'r') as f:
        content = f.read()
    return content


class OpenhandsRunner:
    def __init__(self, model_name, prompt_type):
        self.log_dir = f"/.openhands/"
        self.prompt_type = prompt_type
        os.makedirs(self.log_dir, exist_ok=True)

        if model_name in MODEL_MAPPINGS:
            self.model_name = MODEL_MAPPINGS[model_name]
        else:
            self.model_name = model_name

        if model_name in OPENAI_NO_REASONING_MODELS or model_name in OPENAI_REASONING_MODELS:
            key = os.environ["OPENAI_API_KEY"]
        elif model_name in CLAUDE_NO_REASONING_MODELS or model_name in CLAUDE_REASONING_MODELS:
            key = os.environ["ANTHROPIC_API_KEY"]
        else:
            raise Exception("Model not supported!")
        os.environ["LLM_API_KEY"] = key

        self.llm_config = LLM(
            model=self.model_name,
            reasoning_effort="medium",
            extended_thinking_budget=8000,
        )

        self.agent_config = Agent(
            llm=self.llm_config,
            tools=get_default_tools(enable_browser=False)
        )

    @staticmethod
    def run_with_timeout(func, timeout, *args, **kwargs):
        result = [None]
        exception = [None]
        completed = [False]

        def worker():
            try:
                result[0] = func(*args, **kwargs)
                completed[0] = True
            except Exception as e:
                exception[0] = e

        thread = threading.Thread(target=worker)
        thread.daemon = True
        thread.start()
        thread.join(timeout)

        if not completed[0]:
            return False, "Timeout occurred"
        if exception[0]:
            return False, str(exception[0])
        return True, result[0]

    @staticmethod
    def init(repo_dir):
        shutil.rmtree(f"{repo_dir}/.git")
        repo = Repo.init(repo_dir)
        return repo

    @staticmethod
    def commit(repo: Repo, file=None):
        if file:
            repo.git.add(file)
        else:
            repo.git.add(A=True)
        staged = repo.git.diff("--cached", "--name-only").strip()
        if not staged:
            return None
        msg = f"Auto-commit on {time.strftime('%Y-%m-%d %H:%M:%S')}"
        new_commit = repo.index.commit(msg)
        return new_commit.hexsha

    @staticmethod
    def diff_between(repo: Repo, base_sha: str, head_sha: str):
        return repo.git.diff(f"{base_sha}..{head_sha}")

    def run(self, system_prompt, repo_folder, changed_file):
        repo_base = self.init(repo_folder)

        replaced_file_path = f"/descriptions/mask_desc_perturbed"
        file_content = get_c_cpp_file(replaced_file_path)
        changed_file_path = f"{repo_folder}/{changed_file}"
        Path(changed_file_path).write_text(file_content)

        mask_id = self.commit(repo_base)

        system_prompt = system_prompt.replace(
            " Only return the code to be filled in the masked region. DO NOT include any other information, such as a preamble or suffix.", "")
        user_prompt = AGENT_USER_PEOMPT.format(changed_file=changed_file)
        prompt = system_prompt + user_prompt

        api_key = os.getenv("LLM_API_KEY")
        assert api_key is not None, "LLM_API_KEY environment variable is not set."

        conversation = Conversation(
            agent=self.agent_config,
            workspace=os.path.abspath(repo_folder),
            persistence_dir=self.log_dir
        )

        conversation.send_message(prompt)

        # Run with timeout
        max_retries = 3
        retry_count = 0
        while retry_count < max_retries:
            success, result = self.run_with_timeout(
                conversation.run, 600)  # 600 secs timeout
            if success or result != "Timeout occurred":
                break
            retry_count += 1
            time.sleep(1)

        if success:
            conversation.close()
            self.commit(repo_base, changed_file)

            with open(changed_file_path) as f:
                content = f.read()
            diff = self.diff_between(repo_base, mask_id, "HEAD")
            return diff, content
        else:
            raise Exception("Patching unsuccessful!")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--model-name', type=str)
    parser.add_argument('--model-alias', type=str)
    parser.add_argument('--prompt-type', type=str)
    parser.add_argument('--system-prompt', type=str)
    parser.add_argument('--repo-folder', type=str)
    parser.add_argument('--changed-file', type=str)
    parser.add_argument('--context-type', type=str)
    parser.add_argument('--mode', type=str)
    args = parser.parse_args()

    client = OpenhandsRunner(args.model_name, args.prompt_type)
    diff, response = client.run(
        args.system_prompt, args.repo_folder, args.changed_file)

    Path(f'/diff/openhands-{args.model_alias}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}.diff').write_text(diff)
    Path(f'/completions/openhands-{args.model_alias}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_code_completion.txt').write_text(response)


if __name__ == "__main__":
    main()