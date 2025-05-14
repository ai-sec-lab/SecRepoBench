import os
import json
import openai
import anthropic
import torch
import backoff
import google
from tqdm import tqdm
import google.generativeai as genai
from google.generativeai import GenerationConfig
from transformers import AutoTokenizer, AutoModelForCausalLM
from abc import ABC, abstractmethod
from typing import List, Dict, Any

os.environ["OPENAI_API_KEY"] = ""
class BaseEvaler(ABC):
    def __init__(self):
        self.model_name = 'gpt-4o-2024-08-06'
        self.cache_file = f"sec_code_plt/cache.json"
        self.responses_cache = self._load_cache()

    def _load_cache(self) -> Dict[str, str]:
        if os.path.exists(self.cache_file):
            with open(self.cache_file, 'r') as f:
                return json.load(f)
        return {}

    @staticmethod
    def postprocess(response: str) -> str:
        return response.strip()

    def _get_prompt(self, id: str) -> str:
        with open(f'sec_code_plt/security_policy/{id}/prompt.txt', 'r') as f:
            prompt = f.read()
        return prompt

    @abstractmethod
    def get_response(self, id: str) -> str:
        pass

    def save_cache(self):
        with open(self.cache_file, 'w') as f:
            json.dump(self.responses_cache, f, indent=4)

class APIEvaler(BaseEvaler):
    def __init__(self):
        super().__init__()
        self.client = self._initialize_client()
        self.create = self._get_create_function()
        self.get_content = self._get_content_function()

    def _initialize_client(self):
        if 'gpt-' in self.model_name:
            return openai.OpenAI()
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_create_function(self):
        if 'gpt-' in self.model_name:
            return self.client.chat.completions.create
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_content_function(self):
        if 'gpt-' in self.model_name:
            return lambda response: [choice.message.content for choice in response.choices]
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _create_messages(self, prompt: str, system_prompt: str) -> List[Dict[str, str]]:
        if 'gpt-' in self.model_name:
            return [
                {'role': 'system', 'content': system_prompt},
                {'role': 'user', 'content': prompt},
            ]
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    def _get_model_kwargs(self, messages: List[Dict[str, str]], system_prompt: str, temperature: float = 0.8, max_tokens: int = 2048) -> Dict[str, Any]:
        if 'gpt-' in self.model_name:
            return {
                'model': self.model_name,
                'messages': messages,
                'temperature': temperature,
                'max_tokens': max_tokens,
                'top_p': 0.95,
                'n': 1,
            }
        else:
            raise ValueError(f'Invalid model name: {self.model_name}')

    @backoff.on_exception(backoff.expo, (openai.RateLimitError, 
                                         openai.InternalServerError, 
                                         openai.APIConnectionError, 
                                         ))
    def get_response(self, id: str) -> str:
        prompt = self._get_prompt(id)
        system_prompt = ("You are a software security expert in C/C++.")

        if id in self.responses_cache:
            print('Using cache')
            return self.postprocess(self.responses_cache[id])

        messages = self._create_messages(prompt, system_prompt)
        kwargs = self._get_model_kwargs(messages, system_prompt)
        
        response = self.create(**kwargs)
        try:
            response = self.get_content(response)[0]
        except Exception as e:
            print(f"{id}: Error: {e}")
            return ""
        self.responses_cache[id] = response

        return self.postprocess(response)
    

def main():
    evaler = APIEvaler()
    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    for id in tqdm(ids):
        response = evaler.get_response(id)
        evaler.save_cache()
        with open(f'sec_code_plt/security_policy/{id}/security_policy.txt', 'w') as f:
            f.write(response)
    evaler.save_cache()

if __name__ == '__main__':
    main()
