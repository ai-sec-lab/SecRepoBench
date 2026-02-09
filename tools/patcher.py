from dotenv import load_dotenv, find_dotenv
import asyncio
import os
import json
import openai
import shutil
import anthropic
import torch
import requests
import backoff
import subprocess
import google
import google.generativeai as genai
from harnesses.claudecode_harness import ClaudeCodeRunner
from google.generativeai import GenerationConfig
from transformers import AutoTokenizer, AutoModelForCausalLM
from assets.constants import *
from assets.cwe_map import *
from itertools import chain
from abc import ABC, abstractmethod
from typing import List, Dict, Any
from tools.utils import get_c_cpp_file


# automatically find & load the nearest .env file
load_dotenv(find_dotenv())


safety_settings = [
    {
        "category": "HARM_CATEGORY_DANGEROUS",
        "threshold": "BLOCK_NONE",
    },
    {
        "category": "HARM_CATEGORY_HARASSMENT",
        "threshold": "BLOCK_NONE",
    },
    {
        "category": "HARM_CATEGORY_HATE_SPEECH",
        "threshold": "BLOCK_NONE",
    },
    {
        "category": "HARM_CATEGORY_SEXUALLY_EXPLICIT",
        "threshold": "BLOCK_NONE",
    },
    {
        "category": "HARM_CATEGORY_DANGEROUS_CONTENT",
        "threshold": "BLOCK_NONE",
    },
]


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


class BasePatcher(ABC):
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        self.model_name = MODELS[model_name]

        self.context_type = context_type
        self.prompt_type = prompt_type
        self.mode = mode

        os.makedirs("./cache", exist_ok=True)
        self.cache_file = f"cache/{model_name}-{self.context_type}-{self.prompt_type}-{self.mode}.json"
        self.base_cache_file = f"cache/{model_name}-{self.context_type}.json"
        self.responses_cache = self._load_cache()

    def _load_cache(self) -> Dict[str, str]:
        if os.path.exists(self.cache_file):
            with open(self.cache_file, 'r') as f:
                return json.load(f)
        return {}

    @staticmethod
    def postprocess(response: str) -> str:
        if '```' in response:
            start = response.find('```')
            start = response.find('\n', start) + 1
            end = response.find('```', start)
            response = response[start:end]
        return response.strip()

    def _get_prompt(self, id: str, mode) -> str:
        if self.context_type == 'in-file':
            with open(f'descriptions/{id}/in-file.txt', 'r') as file:
                context = file.read()
            return INFILE_PROMPT.format(context=context.strip())
        elif self.context_type == 'in-file-truncated':
            file_path = f'descriptions/{id}/in-file-truncated.txt'
            if not os.path.exists(file_path):
                print(f"File not found: {file_path}. Skipping this prompt.")
                return ""
            with open(file_path, 'r') as file:
                context = file.read()

            return INFILE_PROMPT.format(context=context.strip())
        elif self.context_type == 'BM25':
            # get func_mask_desc
            context1 = get_c_cpp_file(
                f'descriptions/{id}/mask_sec_func_desc_{mode}')
            with open(f'descriptions/{id}/BM25.txt', 'r') as file:
                context2 = file.read()
            return CROSS_FILE_PROMPT.format(context1=context1.strip(), context2=context2.strip())
        elif self.context_type == 'func':
            mask_func_desc = f"descriptions/{id}/mask_sec_func_desc_{mode}"
            if os.path.exists(mask_func_desc + '.c'):
                path = mask_func_desc + '.c'
            elif os.path.exists(mask_func_desc + '.cpp'):
                path = mask_func_desc + '.cpp'
            else:
                print(f'ID {id}: mask_sec_func_desc_{mode} file not present')
                return
            with open(path, 'r') as f:
                context = f.read()
            return FUNC_PROMPT.format(context=context.strip())
        elif self.context_type == 'dense-file':
            # get func_mask_desc
            context1 = get_c_cpp_file(
                f'descriptions/{id}/mask_sec_func_desc_{mode}')
            with open(f'descriptions/{id}/dense-file.txt', 'r') as file:
                context2 = file.read()
            return CROSS_FILE_PROMPT.format(context1=context1.strip(), context2=context2.strip())
        else:
            raise ValueError(f"Invalid context type: {self.context_type}")

    def _get_system_prompt(self, id):
        if self.prompt_type == 'sec-generic':
            system_prompt = SEC_GENERIC_PROMPT
        elif self.prompt_type == 'sec-practice':
            system_prompt = SEC_PRACTICE_PROMPT
        elif self.prompt_type == 'sec-specific':
            cwe_id, cwe_description = get_cwe_info(id)
            system_prompt = SEC_SPECIFIC_PROMPT.format(
                CWE_ID=cwe_id, CWE_description=cwe_description)
        elif self.prompt_type == 'no-security-reminder':
            system_prompt = SYSTEM_PROMPT
        elif self.prompt_type == 'security-policy':
            with open(f'./security_policy/{id}/security_policy.txt', 'r') as f:
                security_policy = f.read()
            system_prompt = SECURITY_POLICY.format(
                security_policy=security_policy)
        else:
            system_prompt = None

        return system_prompt

    @abstractmethod
    def get_response(self, id: str, mode: str, rerun: bool) -> str:
        pass

    def save_cache(self):
        with open(self.cache_file, 'w') as f:
            json.dump(self.responses_cache, f)


class AgentPatcher(BasePatcher):
    def __init__(self, agent: str, model_name: str, context_type: str, prompt_type: str, mode: str):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.agent = agent
        self.model_alias = model_name
        self.cache_file = f"cache/{self.agent}-{self.model_alias}-{self.context_type}-{self.prompt_type}-{self.mode}.json"
        self.base_cache_file = f"cache/{self.agent}-{self.model_alias}-{self.context_type}.json"
        self.log_dir = f".{self.agent}/{self.model_alias}/{self.prompt_type}"

        os.makedirs(self.log_dir, exist_ok=True)

    def get_response(self, id: str, mode: str, rerun: bool) -> str:
        prompt = ""
        system_prompt = self._get_system_prompt(id)

        if not rerun and os.path.exists(f'./diff/{id}/{self.agent}-{self.model_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}.diff') and os.path.exists(f'./completions/{id}/{self.agent}-{self.model_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}_code_completion.txt'):
            print(f'Using cache for {id}')
        else:
            try:
                self.run_docker(id, system_prompt)
            except Exception as e:
                print(f"{id}: Error: {e}")
                return "", prompt, system_prompt

        with open(f'./completions/{id}/{self.agent}-{self.model_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}_code_completion.txt') as f:
            response = f.read()
        return response, prompt, system_prompt

    def run_docker(self, id: str, system_prompt: str):
        base_dir = os.getcwd()
        sample_metadata = json.load(open('sample_metadata.json', "r"))
        project_name = sample_metadata[id]["project_name"]
        changed_file = sample_metadata[id]["changed_file"]

        container_id = f"{id}_{self.agent}_{self.model_name}_{self.context_type}_{self.prompt_type}_{self.mode}"

        uid = os.getuid()
        env_vars = {
            "MAKEFLAGS": "-j8",
            "OPENAI_API_KEY": os.getenv("OPENAI_API_KEY"),
            "ANTHROPIC_API_KEY": os.getenv("ANTHROPIC_API_KEY"),
            "GEMINI_API_KEY": os.getenv("GEMINI_API_KEY"),
        }
        env_flags = " ".join(
            [f'-e {key}="{value}"' for key, value in env_vars.items()])
        if os.path.exists(f"{self.log_dir}/{id}"):
            shutil.rmtree(f"{self.log_dir}/{id}")
        
        os.makedirs(f"./diff/{id}", exist_ok=True)

        volumes = [
            f"{base_dir}/harnesses:/harnesses",
            f"{base_dir}/assets/constants.py:/workdir/constants.py",
            f"{base_dir}/diff/{id}:/diff",
            f"{base_dir}/completions/{id}:/completions",
            f"{base_dir}/{self.log_dir}/{id}:/.{self.agent}",
        ]
        volume_flags = " ".join([f"-v {vol}" for vol in volumes])
        
        install_cmd = AGENT_INSTALL_COMMANDS[self.agent]

        content = (
            "#!/bin/bash\n"
            "docker run --rm --init "
            f"--name {container_id} "
            "--cpus=8 "
            f"{env_flags} "
            f"{volume_flags} "
            f"secrepobench:{id} /bin/sh -c \"\n"
            # limit num processes to 8 by changing nproc behavior
            "  echo '#!/bin/sh' > /tmp/nproc\n"
            "  echo 'echo 8' >> /tmp/nproc\n"
            "  chmod +x /tmp/nproc\n"
            "  export PATH=/tmp:$PATH\n"
            # install agent dependencies
            "  cp -r /harnesses/* /workdir\n"
            f"  GIT_DIR=\\$(find /src -type d -iname '{project_name}' | head -n 1)\n"
            "  cd /workdir\n"
            "  . /root/.local/bin/env\n"
            "  uv python install 3.12\n"
            "  uv venv --python 3.12\n"
            f"{install_cmd}"
            # run inference
            f"  uv run {self.agent}_harness.py --model-name {self.model_name} --model-alias {self.model_alias} --prompt-type {self.prompt_type} --repo-folder \\$GIT_DIR "
            f"--system-prompt \\\"{system_prompt}\\\" --changed-file {changed_file} --context-type {self.context_type} --mode {self.mode} \n"
            f"  chown -R {uid}:{uid} /diff /completions /.{self.agent}\n"
            "  \""
        )

        bash_dir = f"{id}/inference_{self.agent}_{self.model_alias}_filled_code_{self.context_type}_{self.prompt_type}_{self.mode}.sh"

        with open(f"./completions/{bash_dir}", "w") as f:
            f.write(content)
            
        output_dir = f"./inference_data/{id}/inference_{self.agent}_{self.model_alias}_filled_code_{self.context_type}_{self.prompt_type}_{self.mode}"
        os.makedirs(output_dir, exist_ok=True)

        stdout_path = os.path.join(output_dir, "stdout.txt")
        stderr_path = os.path.join(output_dir, "stderr.txt")

        with open(stdout_path, 'w') as stdout_file, open(stderr_path, 'w') as stderr_file:
            subprocess.run(['/bin/bash', f"./completions/{bash_dir}"],
                        check=True,
                        stdout=stdout_file,
                        stderr=stderr_file)

        subprocess.run(['docker', 'rm', '-f', container_id],
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)


class ClaudeCodePatcher(BasePatcher):
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.cache_file = f"cache/claudecode-{model_name}-{self.context_type}-{self.prompt_type}-{self.mode}.json"
        self.base_cache_file = f"cache/claudecode-{model_name}-{self.context_type}.json"
        self.model_name_alias = model_name
        self.client = ClaudeCodeRunner(self.model_name, prompt_type)

    def get_response(self, id: str, mode: str, rerun: bool) -> str:
        prompt = ""
        system_prompt = self._get_system_prompt(id)

        if not rerun and os.path.exists(f'./diff/{id}/claudecode-{self.model_name_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}.diff') and os.path.exists(f'./completions/{id}/claudecode-{self.model_name_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}_code_completion.txt'):
            print(f'Using cache for {id}')
            with open(f'./completions/{id}/claudecode-{self.model_name_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}_code_completion.txt') as f:
                response = f.read()
                return response, prompt, system_prompt
        try:
            diff, response = asyncio.run(self.client.run(system_prompt, id))
            os.makedirs(f"./diff/{id}", exist_ok=True)
            with open(f'./diff/{id}/claudecode-{self.model_name_alias}-filled-code-{self.context_type}-{self.prompt_type}-{mode}.diff', "w") as f:
                f.write(diff)
        except Exception as e:
            print(f"{id}: Error: {e}")
            return "", prompt, system_prompt

        return response, prompt, system_prompt


class APIPatcher(BasePatcher):
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.client = self._initialize_client()
        self.create = self._get_create_function()
        self.get_content = self._get_content_function()

    def _initialize_client(self, system_prompt=None):
        if self.model_name in OPENAI_NO_REASONING_MODELS or self.model_name in OPENAI_REASONING_MODELS or self.model_name in OPENAI_RESPONSE_MODELS:
            return openai.OpenAI()
        elif self.model_name in CLAUDE_NO_REASONING_MODELS or self.model_name in CLAUDE_REASONING_MODELS:
            return anthropic.Anthropic()
        elif self.model_name in GEMINI_NO_REASONING_MODELS:
            genai.configure(api_key=os.environ["GOOGLE_API_KEY"])
            if system_prompt is not None:
                return genai.GenerativeModel(model_name=self.model_name,
                                             system_instruction=system_prompt,
                                             )
            else:
                return genai.GenerativeModel(model_name=self.model_name)
        elif 'qwen-' in self.model_name:
            return openai.OpenAI(
                api_key=os.getenv("QWEN_API_KEY"),
                base_url="https://dashscope-intl.aliyuncs.com/compatible-mode/v1"
            )
        elif self.model_name in TOGETHER_AI_REASONING_MODLES or self.model_name in TOGETHER_AI_NO_REASONING_MODLES:

            together_api_key = os.environ.get("TOGETHER_API_KEY")
            if not together_api_key:
                raise ValueError("TOGETHER_API_KEY not set in environment")
            # Return a simple dict; our create function will use this key.
            return {"api_key": together_api_key}
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_create_function(self):
        if self.model_name in OPENAI_NO_REASONING_MODELS or self.model_name in OPENAI_REASONING_MODELS:
            return self.client.chat.completions.create
        elif self.model_name in OPENAI_RESPONSE_MODELS:
            return self.client.responses.create
        elif self.model_name in CLAUDE_NO_REASONING_MODELS or self.model_name in CLAUDE_REASONING_MODELS:
            return self.client.messages.create
        elif self.model_name in GEMINI_NO_REASONING_MODELS:
            return self.client.generate_content
        elif 'qwen-' in self.model_name:
            return self.client.chat.completions.create
        elif self.model_name in TOGETHER_AI_REASONING_MODLES or self.model_name in TOGETHER_AI_NO_REASONING_MODLES:
            # Define a function that calls the Together API endpoint.
            def together_create(**kwargs):
                url = "https://api.together.xyz/v1/chat/completions"
                headers = {
                    "Authorization": f"Bearer {os.environ['TOGETHER_API_KEY']}",
                    "Content-Type": "application/json",
                }
                response = requests.post(url, headers=headers, json=kwargs)
                response.raise_for_status()
                return response.json()
            return together_create
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_content_function(self):
        if self.model_name in OPENAI_NO_REASONING_MODELS or self.model_name in OPENAI_REASONING_MODELS:
            return lambda response: [choice.message.content for choice in response.choices]
        elif self.model_name in OPENAI_RESPONSE_MODELS:
            return lambda response: [j.text for j in list(chain(*[i.content for i in response.output if i.type == "message"])) if j.type == "output_text"]
        elif self.model_name in CLAUDE_REASONING_MODELS:
            return lambda response: [response.content[1].text]
        elif self.model_name in CLAUDE_NO_REASONING_MODELS:
            return lambda response: [content.text for content in response.content]
        elif 'gemini-' in self.model_name:
            return lambda response: [response.text]
        elif 'qwen-' in self.model_name:
            return lambda response: [choice.message.content for choice in response.choices]
        elif self.model_name in TOGETHER_AI_REASONING_MODLES or self.model_name in TOGETHER_AI_NO_REASONING_MODLES:
            return lambda response: [response['choices'][0]['message']['content']]
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _create_messages(self, prompt: str, system_prompt: str, history=[]) -> List[Dict[str, str]]:
        if self.model_name in OPENAI_NO_REASONING_MODELS or self.model_name in OPENAI_REASONING_MODELS:
            if system_prompt is not None:
                messages = [
                    {'role': 'system', 'content': system_prompt},
                ]
            else:
                messages = []
            messages.extend([
                *[{'role': role, 'content': content}
                    for role, content in history],
                {'role': 'user', 'content': prompt},
            ])
        elif self.model_name in OPENAI_RESPONSE_MODELS:
            if system_prompt is not None:
                messages = [
                    {'role': 'developer', 'content': system_prompt},
                ]
            else:
                messages = []
            messages.extend([
                *[{'role': role, 'content': content}
                    for role, content in history],
                {'role': 'user', 'content': prompt},
            ])
        elif self.model_name in CLAUDE_NO_REASONING_MODELS or self.model_name in CLAUDE_REASONING_MODELS:
            messages = [
                *[{'role': role, 'content': content}
                    for role, content in history],
                {'role': 'user', 'content': prompt},
            ]
        elif self.model_name in GEMINI_NO_REASONING_MODELS:
            messages = [
                *[{'role': role.replace("assistant", "model"), 'parts': [content]}
                  for role, content in history],
                {'role': 'user', 'parts': [prompt]}
            ]
        elif self.model_name in GEMINI_NO_REASONING_MODELS:
            if system_prompt is not None:
                messages = [
                    {'role': 'system', 'content': system_prompt},
                ]
            else:
                messages = []
            messages.extend([
                *[{'role': role, 'content': content}
                    for role, content in history],
                {'role': 'user', 'content': prompt},
            ])
        elif self.model_name in TOGETHER_AI_REASONING_MODLES or self.model_name in TOGETHER_AI_NO_REASONING_MODLES:
            messages = [{'role': 'system', 'content': system_prompt}
                        ] if system_prompt else []
            messages.extend([{'role': role, 'content': content}
                            for role, content in history])
            messages.append({'role': 'user', 'content': prompt})

        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

        return messages

    def _get_model_kwargs(self, messages: List[Dict[str, str]], system_prompt: str, temperature: float = 0, max_tokens: int = 3072) -> Dict[str, Any]:
        if self.model_name in OPENAI_NO_REASONING_MODELS:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_completion_tokens': max_tokens,
                'top_p': 1,
                'n': 1,
            }

        # reasoning models
        elif self.model_name in OPENAI_REASONING_MODELS:
            return {
                'model': self.model_name,
                'messages': messages,
                'max_completion_tokens': max_tokens + THINKING_BUDGET_TOKENS,
                'reasoning_effort': 'medium',
                'top_p': 1,
                'n': 1,
            }
        elif self.model_name in OPENAI_RESPONSE_MODELS:
            return {
                'model': self.model_name,
                'tool_choice': 'none',
                'input': messages,
                'reasoning': {
                    'effort': 'medium'
                },
                'max_output_tokens': max_tokens + THINKING_BUDGET_TOKENS,
                'top_p': 1,
            }

        elif self.model_name in CLAUDE_REASONING_MODELS:
            if system_prompt is None:
                return {
                    'model': self.model_name,
                    'messages': messages,
                    'max_tokens': max_tokens + THINKING_BUDGET_TOKENS,
                    'thinking': {
                        "type": "enabled",
                        "budget_tokens": THINKING_BUDGET_TOKENS
                    },
                    'top_p': 1,
                }
            return {
                'model': self.model_name,
                'messages': messages,
                'max_tokens': max_tokens + THINKING_BUDGET_TOKENS,
                'system': system_prompt,
                'thinking': {
                    "type": "enabled",
                    "budget_tokens": THINKING_BUDGET_TOKENS
                },
                'top_p': 1,
            }
        elif self.model_name in CLAUDE_NO_REASONING_MODELS:
            if system_prompt is None:
                return {
                    'model': self.model_name,
                    'messages': messages,
                    'temperature': temperature,
                    'max_tokens': max_tokens,
                    'top_p': 1,
                }
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': max_tokens,
                'top_p': 1,
                'system': system_prompt,
            }
        elif self.model_name in GEMINI_NO_REASONING_MODELS:
            config = GenerationConfig(
                max_output_tokens=max_tokens,
                temperature=temperature,
                top_p=1,
            )
            return {
                'contents': messages,
                'generation_config': config,
                'safety_settings': safety_settings,
            }
        elif 'qwen-' in self.model_name:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': max_tokens,
                'top_p': 1,
                'n': 1,
            }
        elif self.model_name in TOGETHER_AI_REASONING_MODLES:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'reasoning_effort': 'medium',
                'max_tokens': max_tokens + THINKING_BUDGET_TOKENS,
                'top_p': 1,
                'n': 1,
            }
        elif self.model_name in TOGETHER_AI_NO_REASONING_MODLES:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': max_tokens,
                'top_p': 1,
                'n': 1,
            }
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    @backoff.on_exception(backoff.expo, (openai.RateLimitError,
                                         openai.InternalServerError,
                                         openai.APIConnectionError,
                                         anthropic.RateLimitError,
                                         anthropic.APIConnectionError,
                                         anthropic.InternalServerError,
                                         google.api_core.exceptions.ResourceExhausted,
                                         google.api_core.exceptions.TooManyRequests,
                                         google.api_core.exceptions.InternalServerError,
                                         requests.exceptions.HTTPError,
                                         requests.exceptions.RequestException))
    def get_response(self, id: str, mode, rerun: bool) -> str:
        prompt = self._get_prompt(id, mode)
        system_prompt = self._get_system_prompt(id)

        if not rerun and id in self.responses_cache:
            print(f'Using cache for {id}')
            return self.postprocess(self.responses_cache[id]), prompt, system_prompt

        if self.model_name in GEMINI_NO_REASONING_MODELS:
            self.client = self._initialize_client(system_prompt)
            self.create = self._get_create_function()

        if self.prompt_type != 'refine':
            messages = self._create_messages(prompt, system_prompt)
            kwargs = self._get_model_kwargs(messages, system_prompt)

            response = self.create(**kwargs)
        else:
            with open(self.base_cache_file, 'r') as f:
                base_cache = json.load(f)
            base_response = self.postprocess(base_cache[id])
            with open(f'descriptions/{id}/new-in-file.txt', 'r') as file:
                context1 = file.read()
            with open(f'descriptions/{id}/BM25.txt', 'r') as file:
                context2 = file.read()
            prompt1 = REFINE_PROMPT_FIRST.format(
                context1=context1.strip(), context2=context2.strip(), solution=base_response)
            messages = self._create_messages(prompt1, system_prompt)
            kwargs = self._get_model_kwargs(messages, system_prompt)
            analysis = self.create(**kwargs)
            analysis = self.get_content(analysis)[0]
            prompt2 = REFINE_PROMPT_SECOND
            messages = self._create_messages(prompt2, system_prompt, history=[
                                             ("user", prompt1), ("assistant", analysis)])
            kwargs = self._get_model_kwargs(messages, system_prompt)
            response = self.create(**kwargs)
        try:
            response = self.get_content(response)[0]
        except Exception as e:
            print(f"{id}: Error: {e}")
            return "", prompt, system_prompt
        self.responses_cache[id] = response

        return self.postprocess(response), prompt, system_prompt


class ChatPatcher(BasePatcher):
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.tokenizer = AutoTokenizer.from_pretrained(
            self.model_name, trust_remote_code=True)
        self.tokenizer.pad_token_id = self.tokenizer.eos_token_id
        self.mode = mode
        self.model = AutoModelForCausalLM.from_pretrained(
            self.model_name,
            torch_dtype=torch.bfloat16,
            attn_implementation="flash_attention_2",
            device_map="auto",
            trust_remote_code=True, cache_dir="/space2/cache")
        self.model.generation_config.pad_token_id = self.tokenizer.pad_token_id
        self.model.eval()

    def get_response(self, id: str, mode, rerun: bool) -> str:
        prompt = self._get_prompt(id, mode)
        system_prompt = self._get_system_prompt(id)

        if not rerun and id in self.responses_cache:
            print(f'Using cache for {id}')
            return self.postprocess(self.responses_cache[id]), prompt, system_prompt

        terminators = [
            self.tokenizer.eos_token_id,
        ]
        # if 'llama' in self.model_name.lower():
        #     terminators.append(self.tokenizer.convert_tokens_to_ids("<|eot_id|>"))

        if self.prompt_type != 'refine':
            if system_prompt is not None:
                messages = [
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": prompt},
                ]
            else:
                messages = [
                    {"role": "user", "content": prompt},
                ]
            input_ids = self.tokenizer.apply_chat_template(
                messages, add_generation_prompt=True, return_tensors="pt", truncation=True, max_length=16384).to(self.model.device)

            outputs = self.model.generate(input_ids,
                                          max_new_tokens=3072,
                                          attention_mask=torch.ones_like(
                                              input_ids),
                                          do_sample=False,
                                          temperature=None,
                                          top_p=None,
                                          num_return_sequences=1,
                                          eos_token_id=terminators,
                                          use_cache=True,
                                          )
        else:
            with open(self.base_cache_file, 'r') as f:
                base_cache = json.load(f)
            base_response = self.postprocess(base_cache[id])
            with open(f'descriptions/{id}/new-in-file.txt', 'r') as file:
                context1 = file.read()
            with open(f'descriptions/{id}/BM25.txt', 'r') as file:
                context2 = file.read()
            prompt1 = REFINE_PROMPT_FIRST.format(
                context1=context1.strip(), context2=context2.strip(), solution=base_response)
            messages = [
                {"role": "user", "content": prompt1},
            ]
            input_ids = self.tokenizer.apply_chat_template(
                messages, add_generation_prompt=True, return_tensors="pt").to(self.model.device)
            outputs = self.model.generate(input_ids,
                                          max_new_tokens=3072,
                                          attention_mask=torch.ones_like(
                                              input_ids),
                                          do_sample=False,
                                          temperature=None,
                                          top_p=None,
                                          num_return_sequences=1,
                                          eos_token_id=terminators,
                                          use_cache=True,
                                          )
            analysis = self.tokenizer.decode(
                outputs[0][len(input_ids[0]):], skip_special_tokens=True)
            prompt2 = REFINE_PROMPT_SECOND
            messages = [
                {"role": "user", "content": prompt1},
                {"role": "assistant", "content": analysis},
                {"role": "user", "content": prompt2},
            ]
            input_ids = self.tokenizer.apply_chat_template(
                messages, add_generation_prompt=True, return_tensors="pt").to(self.model.device)
            outputs = self.model.generate(input_ids,
                                          max_new_tokens=3072,
                                          attention_mask=torch.ones_like(
                                              input_ids),
                                          do_sample=False,
                                          temperature=None,
                                          top_p=None,
                                          num_return_sequences=1,
                                          eos_token_id=terminators,
                                          use_cache=True,
                                          )

        response = self.tokenizer.decode(
            outputs[0][len(input_ids[0]):], skip_special_tokens=True)
        self.responses_cache[id] = response

        return self.postprocess(response), prompt, system_prompt


class CosecPatcher(BasePatcher):
    def __init__(self, model_name: str, final_model_path: str, context_type: str, prompt_type: str, mode: str, args):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.args = args  # Save the command-line arguments
        # Load specialized model (for example, CodeLlamaModelLM)
        self.model = CodeLlamaModelLM.from_pretrained(
            self.model_name, torch_dtype=torch.bfloat16, device_map='auto')
        # Load expert (final) model
        self.sec_model = AutoModelForCausalLM.from_pretrained(
            final_model_path, torch_dtype=torch.bfloat16, attn_implementation="flash_attention_2",  device_map='auto')

        # Load tokenizer from the specialized model path
        self.tokenizer = AutoTokenizer.from_pretrained(
            self.model_name, trust_remote_code=True)
        self.tokenizer.pad_token_id = self.tokenizer.eos_token_id

        # Set models to evaluation mode
        self.model.eval()
        self.sec_model.eval()

    def get_response(self, id: str, mode, rerun: bool) -> str:
        prompt = self._get_prompt(id, mode)
        system_prompt = self._get_system_prompt(id)

        if not rerun and id in self.responses_cache:
            print(f'Using cache for {id}')
            return self.postprocess(self.responses_cache[id]), prompt, system_prompt

        if not prompt.strip():
            print(f"ID {id}: prompt is empty, skipping.")
            return "", prompt, None

        terminators = [
            self.tokenizer.eos_token_id,
        ]
        if 'llama' in self.model_name.lower():
            terminators.append(
                self.tokenizer.convert_tokens_to_ids("<|eot_id|>"))

        if system_prompt is not None:
            messages = [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": prompt},
            ]
        else:
            messages = [
                {"role": "user", "content": prompt},
            ]
        input_ids = self.tokenizer.apply_chat_template(
            messages, add_generation_prompt=True, return_tensors="pt", truncation=True
        ).to(self.model.device)
        print("Input IDs shape:", input_ids.shape)

        # Create kwargs for expert generation using values from self.args
        extra_kwargs = {
            'expert': True,
            'expert_lm': self.sec_model,
            'model_kwargs_expert': {},
            'threshold': self.args.threshold,  # use threshold from args
        }
        import time
        generation_results = {}

        print(len(input_ids[0]))
        start_time = time.time()
        gen_output = self.model.generate_with_experts(
            input_ids=input_ids,
            do_sample=True,
            num_return_sequences=1,
            temperature=self.args.temp,
            max_new_tokens=400,
            top_p=0.95,
            pad_token_id=self.tokenizer.pad_token_id,
            use_cache=True,
            expert_min_prob=0.0,
            expert_temperature=0.4,
            expert_top_p=0.95,
            **extra_kwargs
        )
        gen_time = time.time() - start_time
        print(f"Generation time: {gen_time:.2f} seconds")

        # Decode the generated output
        response = self.tokenizer.decode(
            gen_output[0], skip_special_tokens=True
        )
        final_response = self.postprocess(response)
        print(final_response)
        # Save the result and generation time for this token limit
        print(
            f"Max new tokens: {len(gen_output[0])}, Generation time: {gen_time:.2f} seconds")

        # Cache and return the response along with prompt details
        self.responses_cache[id] = final_response
        return final_response, prompt, system_prompt
