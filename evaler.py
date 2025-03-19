import os
import json
import openai
import anthropic
import torch
import backoff
import google
import google.generativeai as genai
from google.generativeai import GenerationConfig
from transformers import AutoTokenizer, AutoModelForCausalLM
from constants import *
from cwe_map import *
from abc import ABC, abstractmethod
from typing import List, Dict, Any


def get_c_cpp_file(base_path: str):
    c_path = base_path + '.c'
    cpp_path = base_path + '.cpp'
    if os.path.exists(c_path):
        path = c_path
    elif os.path.exists(cpp_path):
        path = cpp_path
    else:
        print(f'This file does not exist with a c or cpp extension: {base_path}')
        return
    with open(path, 'r') as f:
        content = f.read()
    return content


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


class BaseEvaler(ABC):
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        self.model_name = MODELS[model_name]
        self.context_type = context_type
        self.prompt_type = prompt_type
        self.mode = mode
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
        elif self.context_type == 'cross-file':
            # get func_mask_desc
            context1 = get_c_cpp_file(f'descriptions/{id}/mask_func_desc_{mode}')
            with open(f'descriptions/{id}/cross-file.txt', 'r') as file:
                context2 = file.read()
            return CROSS_FILE_PROMPT.format(context1=context1.strip(), context2=context2.strip())
        elif self.context_type == 'func':
            mask_func_desc = f"descriptions/{id}/mask_func_desc_{mode}"
            if os.path.exists(mask_func_desc + '.c'):
                path = mask_func_desc + '.c'
            elif os.path.exists(mask_func_desc + '.cpp'):
                path = mask_func_desc + '.cpp'
            else:
                print(f'ID {id}: mask_func_desc_{mode} file not present')
                return
            with open(path, 'r') as f:
                context = f.read()
            return FUNC_PROMPT.format(context=context.strip())
        else:
            raise ValueError(f"Invalid context type: {self.context_type}")
    
    def _get_system_prompt(self, id):
        if self.prompt_type == 'sec-generic':
            system_prompt = SEC_GENERIC_PROMPT
        elif self.prompt_type == 'sec-practice':
            system_prompt = SEC_PRACTICE_PROMPT
        elif self.prompt_type == 'sec-specific':
            cwe_id, cwe_description = get_cwe_info(id)
            system_prompt = SEC_SPECIFIC_PROMPT.format(CWE_ID=cwe_id, CWE_description=cwe_description)
        elif self.prompt_type == 'system-prompt':
            system_prompt = SYSTEM_PROMPT
        else:
            system_prompt = None

        return system_prompt

    @abstractmethod
    def get_response(self, id: str) -> str:
        pass

    def save_cache(self):
        with open(self.cache_file, 'w') as f:
            json.dump(self.responses_cache, f)

class APIEvaler(BaseEvaler):
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.client = self._initialize_client()
        self.create = self._get_create_function()
        self.get_content = self._get_content_function()

    def _initialize_client(self, system_prompt=None):
        if 'gpt-' in self.model_name:
            return openai.OpenAI()
        elif 'claude-' in self.model_name:
            return anthropic.Anthropic()
        elif 'gemini-' in self.model_name:
            genai.configure(api_key=os.environ["GOOGLE_API_KEY"])
            if system_prompt is not None:
                return genai.GenerativeModel(model_name=self.model_name,
                                            system_instruction=system_prompt,
                                            )
            else:
                return genai.GenerativeModel(model_name=self.model_name)
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_create_function(self):
        if 'gpt-' in self.model_name:
            return self.client.chat.completions.create
        elif 'claude-' in self.model_name:
            return self.client.messages.create
        elif 'gemini-' in self.model_name:
            return self.client.generate_content
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_content_function(self):
        if 'gpt-' in self.model_name:
            return lambda response: [choice.message.content for choice in response.choices]
        elif 'claude-' in self.model_name:
            return lambda response: [content.text for content in response.content]
        elif 'gemini-' in self.model_name:
            return lambda response: [response.text]
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _create_messages(self, prompt: str, system_prompt: str, history=[]) -> List[Dict[str, str]]:
        if 'gpt-' in self.model_name:
            if system_prompt is not None:
                messages = [
                    {'role': 'system', 'content': system_prompt},
                ]
            else:
                messages = []
            messages.extend([
                *[{'role':role, 'content':content} for role, content in history],
                {'role': 'user', 'content': prompt},
            ])
        elif 'claude-' in self.model_name:
            messages = [
                *[{'role':role, 'content':content} for role, content in history],
                {'role': 'user', 'content': prompt},
            ]
        elif 'gemini-' in self.model_name:
            messages = [
                *[{'role':role.replace("assistant", "model"), 'parts':[content]} for role, content in history],
                {'role':'user', 'parts': [prompt]}
            ]
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')
        
        return messages

    def _get_model_kwargs(self, messages: List[Dict[str, str]], system_prompt: str, temperature: float = 0, max_tokens: int = 3072) -> Dict[str, Any]:
        if 'gpt-' in self.model_name:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': max_tokens,
                'top_p': 1,
                'n': 1,
            }
        elif 'claude-' in self.model_name:
            if system_prompt is None:
                return {
                    'model': self.model_name,
                    'messages': messages,
                    'temperature': temperature,
                    'max_tokens': max_tokens,
                    'top_p': 1,
                }
            else:
                return {
                    'model': self.model_name,
                    'messages': messages,
                    'temperature': temperature,
                    'max_tokens': max_tokens,
                    'top_p': 1,
                    'system': system_prompt,
                }
        elif 'gemini-' in self.model_name:
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
                                         google.api_core.exceptions.InternalServerError))
    def get_response(self, id: str, mode) -> str:
        prompt = self._get_prompt(id, mode)
        system_prompt = self._get_system_prompt(id)
        if 'gemini-' in self.model_name:
            self.client = self._initialize_client(system_prompt)
            self.create = self._get_create_function()
        # if id in self.responses_cache:
        #     print('Using cache')
        #     return self.postprocess(self.responses_cache[id]), prompt, system_prompt

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
            with open(f'descriptions/{id}/cross-file.txt', 'r') as file:
                context2 = file.read()
            prompt1 = REFINE_PROMPT_FIRST.format(context1=context1.strip(), context2=context2.strip(), solution=base_response)
            messages = self._create_messages(prompt1, system_prompt)
            kwargs = self._get_model_kwargs(messages, system_prompt)
            analysis = self.create(**kwargs)
            analysis = self.get_content(analysis)[0]
            prompt2 = REFINE_PROMPT_SECOND
            messages = self._create_messages(prompt2, system_prompt, history=[("user", prompt1), ("assistant", analysis)])
            kwargs = self._get_model_kwargs(messages, system_prompt)
            response = self.create(**kwargs)
        try:
            response = self.get_content(response)[0]
        except Exception as e:
            print(f"{id}: Error: {e}")
            return "", prompt, system_prompt
        self.responses_cache[id] = response

        return self.postprocess(response), prompt, system_prompt


class ChatEvaler(BaseEvaler):
    def __init__(self, model_name: str, context_type: str, prompt_type: str):
        super().__init__(model_name, context_type, prompt_type)
        self.tokenizer = AutoTokenizer.from_pretrained(self.model_name, trust_remote_code=True)
        self.tokenizer.pad_token_id = self.tokenizer.eos_token_id
        self.model = AutoModelForCausalLM.from_pretrained(
            self.model_name, 
            torch_dtype=torch.bfloat16, 
            attn_implementation="flash_attention_2", 
            device_map="auto",
            trust_remote_code=True)
        self.model.generation_config.pad_token_id = self.tokenizer.pad_token_id
        self.model.eval()

    def get_response(self, id: str) -> str:
        if id in self.responses_cache:
            print('Using cache')
            return self.postprocess(self.responses_cache[id])
        
        prompt = self._get_prompt(id)
        system_prompt = self._get_system_prompt(id)
        terminators = [
            self.tokenizer.eos_token_id,
        ]
        if 'llama' in self.model_name.lower():
            terminators.append(self.tokenizer.convert_tokens_to_ids("<|eot_id|>"))

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
            input_ids = self.tokenizer.apply_chat_template(messages, add_generation_prompt=True, return_tensors="pt").to(self.model.device)
            outputs = self.model.generate(input_ids,
                                        max_new_tokens=3072,
                                        attention_mask=torch.ones_like(input_ids),
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
            with open(f'descriptions/{id}/cross-file.txt', 'r') as file:
                context2 = file.read()
            prompt1 = REFINE_PROMPT_FIRST.format(context1=context1.strip(), context2=context2.strip(), solution=base_response)
            messages = [
                {"role": "user", "content": prompt1},
            ]
            input_ids = self.tokenizer.apply_chat_template(messages, add_generation_prompt=True, return_tensors="pt").to(self.model.device)
            outputs = self.model.generate(input_ids,
                                        max_new_tokens=3072,
                                        attention_mask=torch.ones_like(input_ids),
                                        do_sample=False,
                                        temperature=None,
                                        top_p=None,
                                        num_return_sequences=1,
                                        eos_token_id=terminators,
                                        use_cache=True,
            )
            analysis = self.tokenizer.decode(outputs[0][len(input_ids[0]):], skip_special_tokens=True)
            prompt2 = REFINE_PROMPT_SECOND
            messages = [
                {"role": "user", "content": prompt1},
                {"role": "assistant", "content": analysis},
                {"role": "user", "content": prompt2},
            ]
            input_ids = self.tokenizer.apply_chat_template(messages, add_generation_prompt=True, return_tensors="pt").to(self.model.device)
            outputs = self.model.generate(input_ids,
                                        max_new_tokens=3072,
                                        attention_mask=torch.ones_like(input_ids),
                                        do_sample=False,
                                        temperature=None,
                                        top_p=None,
                                        num_return_sequences=1,
                                        eos_token_id=terminators,
                                        use_cache=True,
            )

        response = self.tokenizer.decode(outputs[0][len(input_ids[0]):], skip_special_tokens=True)
        self.responses_cache[id] = response

        return self.postprocess(response)