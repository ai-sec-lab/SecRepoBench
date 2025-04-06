import os
import json
import openai
import anthropic
import torch
import requests
import backoff
import google
import google.generativeai as genai
from google.generativeai import GenerationConfig
from transformers import AutoTokenizer, AutoModelForCausalLM
from constants import *
from cwe_map import *
from abc import ABC, abstractmethod
from typing import List, Dict, Any
# from CustomizedGeneration import *

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
            context1 = get_c_cpp_file(f'descriptions/{id}/mask_sec_func_desc_{mode}')
            with open(f'descriptions/{id}/cross-file.txt', 'r') as file:
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
        elif self.prompt_type == 'security-policy':
            with open(f'sec_code_plt/security_policy/{id}/security_policy.txt', 'r') as f:
                security_policy = f.read()
            system_prompt = SECURITY_POLICY.format(security_policy=security_policy)
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
        if 'gpt-' in self.model_name or self.model_name in ['o3-mini-2025-01-31', 'o1-2024-12-17']:
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
        elif 'qwen-' in self.model_name:
            return openai.OpenAI(
                api_key=os.getenv("QWEN_API_KEY"),
                base_url="https://dashscope-intl.aliyuncs.com/compatible-mode/v1"
            )
        elif 'DeepSeek' in self.model_name:
            together_api_key = os.environ.get("TOGETHER_API_KEY")
            if not together_api_key:
                raise ValueError("TOGETHER_API_KEY not set in environment")
            # Return a simple dict; our create function will use this key.
            return {"api_key": together_api_key}
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_create_function(self):
        if 'gpt-' in self.model_name or self.model_name in ['o3-mini-2025-01-31', 'o1-2024-12-17']:
            return self.client.chat.completions.create
        elif self.model_name == 'claude-3-7-sonnet-20250219':
            return self.client.beta.messages.create
        elif 'claude-' in self.model_name:
            return self.client.messages.create
        elif 'gemini-' in self.model_name:
            return self.client.generate_content
        elif 'qwen-' in self.model_name:
            return self.client.chat.completions.create
        elif 'DeepSeek' in self.model_name:
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
        if 'gpt-' in self.model_name or self.model_name in ['o3-mini-2025-01-31', 'o1-2024-12-17']:
            return lambda response: [choice.message.content for choice in response.choices]
        elif self.model_name == 'claude-3-7-sonnet-20250219':
            return lambda response: [response.content[1].text]
        elif 'claude-' in self.model_name:
            return lambda response: [content.text for content in response.content]
        elif 'gemini-' in self.model_name:
            return lambda response: [response.text]
        elif 'qwen-' in self.model_name:
            return lambda response: [choice.message.content for choice in response.choices]
        elif 'DeepSeek' in self.model_name:
            # Assume Together API returns a JSON with choices similar to OpenAI.
            return lambda response: [response['choices'][0]['message']['content']]
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _create_messages(self, prompt: str, system_prompt: str, history=[]) -> List[Dict[str, str]]:
        if 'gpt-' in self.model_name or self.model_name in ['o3-mini-2025-01-31', 'o1-2024-12-17']:
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
        elif 'qwen-' in self.model_name:
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
        elif 'DeepSeek' in self.model_name:
            # Together API uses the same message structure as OpenAI
            messages = [{'role': 'system', 'content': system_prompt}] if system_prompt else []
            messages.extend([{'role': role, 'content': content} for role, content in history])
            messages.append({'role': 'user', 'content': prompt})
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
        elif self.model_name in ['o3-mini-2025-01-31', 'o1-2024-12-17']:
            thinking_budget_tokens = 8_000
            return {
                'model': self.model_name,
                'messages': messages,
                'max_completion_tokens': max_tokens + thinking_budget_tokens,
                'top_p': 1,
                'n': 1,
            }
        elif self.model_name == 'claude-3-7-sonnet-20250219':
            thinking_budget_tokens = 8_000
            return {
                'model': self.model_name,
                'messages': messages,
                'max_tokens': max_tokens + thinking_budget_tokens,
                'system': system_prompt,
                'thinking': {
                    "type": "enabled",
                    "budget_tokens": thinking_budget_tokens
                },
                'betas': ["output-128k-2025-02-19"]
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
        elif 'qwen-' in self.model_name:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': max_tokens,
                'top_p': 1,
                'n': 1,
            }
        elif 'DeepSeek' in self.model_name:
            
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': 8000+max_tokens,
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
                                         requests.exceptions.RequestException  ))
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
    def __init__(self, model_name: str, context_type: str, prompt_type: str, mode: str):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.tokenizer = AutoTokenizer.from_pretrained(self.model_name, trust_remote_code=True)
        self.tokenizer.pad_token_id = self.tokenizer.eos_token_id
        self.mode = mode
        self.model = AutoModelForCausalLM.from_pretrained(
            self.model_name, 
            torch_dtype=torch.bfloat16, 
            attn_implementation="flash_attention_2", 
            device_map="auto",
            trust_remote_code=True, cache_dir = "/space2/cache")
        self.model.generation_config.pad_token_id = self.tokenizer.pad_token_id
        self.model.eval()

    def get_response(self, id: str, mode) -> str:
        prompt = self._get_prompt(id,mode)
        system_prompt = self._get_system_prompt(id)
        if id in self.responses_cache:
            print('Using cache')
            return self.postprocess(self.responses_cache[id]), prompt, system_prompt
        
        
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
            input_ids = self.tokenizer.apply_chat_template(messages, add_generation_prompt=True, return_tensors="pt", truncation=True,max_length=16384).to(self.model.device)
             
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

        return self.postprocess(response), prompt, system_prompt


class CosecEvaler(BaseEvaler):
    def __init__(self, model_name: str, final_model_path: str, context_type: str, prompt_type: str, mode: str, args):
        super().__init__(model_name, context_type, prompt_type, mode)
        self.args = args  # Save the command-line arguments
        # Load specialized model (for example, CodeLlamaModelLM)
         
        self.model = CodeLlamaModelLM.from_pretrained(model_name,torch_dtype=torch.bfloat16, device_map='auto')
        # Load expert (final) model
        self.sec_model = AutoModelForCausalLM.from_pretrained(final_model_path,torch_dtype=torch.bfloat16,attn_implementation="flash_attention_2",  device_map='auto')
        
        # Load tokenizer from the specialized model path
        self.tokenizer = AutoTokenizer.from_pretrained(model_name, trust_remote_code=True)
        self.tokenizer.pad_token_id = self.tokenizer.eos_token_id
        
        # Set models to evaluation mode
        self.model.eval()
        self.sec_model.eval()

    def get_response(self, id: str, mode) -> str:
        prompt = self._get_prompt(id, mode)
        system_prompt = self._get_system_prompt(id)
        if id in self.responses_cache:
            print('Using cache')
            return self.postprocess(self.responses_cache[id]), prompt, system_prompt
        
        terminators = [
            self.tokenizer.eos_token_id,
        ]
        if 'llama' in self.model_name.lower():
            terminators.append(self.tokenizer.convert_tokens_to_ids("<|eot_id|>"))
        
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
            messages, add_generation_prompt=True, return_tensors="pt"
        ).to(self.model.device)
        print("Input IDs shape:", input_ids.shape)
         
        # Create kwargs for expert generation using values from self.args
        extra_kwargs = {
            'expert': True,
            'expert_lm': self.sec_model,
            'model_kwargs_expert': {},
            'threshold': self.args.threshold,  # use threshold from args
        }
         
        gen_output = self.model.generate_with_experts(
            input_ids=input_ids,
            do_sample=True,
            num_return_sequences=1,
            temperature=self.args.temp,
            max_new_tokens=20,
            top_p=0.95,
            pad_token_id=self.tokenizer.pad_token_id,
            use_cache=True,
            expert_min_prob=0.0,
            expert_temperature=0.4,
            expert_top_p=0.95,
            **extra_kwargs
        )
        
        # Decode the generated output
        response = self.tokenizer.decode(gen_output[0][len(input_ids[0]):], skip_special_tokens=True)
        self.responses_cache[id] = response

        return self.postprocess(response), prompt, system_prompt
