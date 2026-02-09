import sys
import os
import json
import time
import sys
import shutil
from pathlib import Path
from git import Repo
import asyncio
from assets.constants import *
from claude_agent_sdk import ClaudeAgentOptions, ClaudeSDKClient, ResultMessage


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


class ClaudeCodeRunner:
    def __init__(self, model_name, prompt_type):
        self.model_name = model_name
        self.agent_config = ClaudeAgentOptions(
            disallowed_tools=["WebSearch", "WebFetch"],
            permission_mode="bypassPermissions",
            model=model_name,
            env={
                "ANTHROPIC_API_KEY": os.environ["ANTHROPIC_API_KEY"],
                "MAX_THINKING_TOKENS": str(8000)
            }
        )
        self.base_dir = f".claudecode/"
        self.prompt_type = prompt_type

    @staticmethod
    async def run_async_with_timeout(async_func, timeout, *args, **kwargs):
        """Native async timeout - more efficient"""
        try:
            result = await asyncio.wait_for(
                async_func(*args, **kwargs),
                timeout=timeout
            )
            return True, result
        except asyncio.TimeoutError:
            return False, "Timeout occurred"
        except Exception as e:
            return False, str(e)

    @staticmethod
    def temp_clone_repo(url, commit, repo_name, id, model):
        if os.path.exists(f"repo_tmp_{model}/{repo_name}_{id}"):
            shutil.rmtree(f"repo_tmp_{model}/{repo_name}_{id}")

        repo = Repo.clone_from(url, f"repo_tmp_{model}/{repo_name}_{id}")
        repo.head.reference = repo.commit(commit)
        repo.head.reset(index=True, working_tree=True)

        return repo, f"repo_tmp_{model}/{repo_name}_{id}"
    
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
        cmd = [
            "git", "diff", base_sha, head_sha,
        ]

        patch_text = repo.git.execute(
            cmd,
            stdout_as_string=True,
            strip_newline_in_stdout=False,
        )

        return patch_text

    @staticmethod
    def clean_repo(repo_folder):
        if os.path.exists(repo_folder):
            shutil.rmtree(repo_folder)

    async def run(self, system_prompt, id):

        log_dir = os.path.join(
            self.base_dir, self.model_name, self.prompt_type, str(id))
        os.makedirs(log_dir, exist_ok=True)

        chat_history_file = Path(log_dir) / 'claudecode-chat-log.jsonl'

        with open("./sample_metadata.json") as f:
            metadata = json.load(f)

        project_name, fixing_commit, changed_file, *_ = metadata[id].values()

        with open("./github_repos.json") as f:
            urls = json.load(f)

        url = None
        for project_meta in urls:
            if project_meta["project"] == project_name:
                url = project_meta["repo_addr"]

        _, repo_folder = self.temp_clone_repo(
            url, fixing_commit, project_name, id, self.model_name)
        
        if repo_folder.startswith("./"):
            repo_folder = repo_folder[2:]
        
        repo_base = self.init(repo_folder)
        
        target_file_path = Path(repo_folder) / changed_file
        replaced_file_path = f"./descriptions/{id}/mask_desc_perturbed"
        file_content = get_c_cpp_file(replaced_file_path)
        target_file_path.write_text(file_content)

        mask_id = self.commit(repo_base)

        self.agent_config.cwd = repo_folder
        async with ClaudeSDKClient(options=self.agent_config) as client:

            system_prompt = system_prompt.replace(
                " Only return the code to be filled in the masked region. DO NOT include any other information, such as a preamble or suffix.", "")
            user_prompt = AGENT_USER_PEOMPT.format(changed_file=changed_file)
            prompt = system_prompt + user_prompt

            # Run with timeout
            max_retries = 3
            retry_count = 0

            success, result = False, None
            while retry_count < max_retries:
                # 1200 secs timeout
                success, result = await self.run_async_with_timeout(client.query, 1200, prompt)
                if success or result != "Timeout occurred":
                    break
                retry_count += 1
                time.sleep(1)  # Brief pause between retries

            async for message in client.receive_response():
                if isinstance(message, ResultMessage):
                    session_id = message.session_id
                    traj_src_path = Path(
                        f"~/.claude/projects/{os.getcwd().replace("/", "-").replace("_", "-")}-{repo_folder.replace("_", "-").replace("/", "-")}/{session_id}.jsonl").expanduser()
                    shutil.copy(traj_src_path, chat_history_file)

            self.commit(repo_base, changed_file)

            if success:
                with open(target_file_path) as f:
                    content = f.read()
                diff = self.diff_between(repo_base, mask_id, "HEAD")
                self.clean_repo(repo_folder)
                return diff, content
            else:
                raise Exception("Patching unsuccessful!")

        time.sleep(0.1)
