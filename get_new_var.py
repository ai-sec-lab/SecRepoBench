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
        self.cache_file = f"cache/desc.json"
        self.responses_cache = self._load_cache()

    def _load_cache(self) -> Dict[str, str]:
        if os.path.exists(self.cache_file):
            with open(self.cache_file, 'r') as f:
                return json.load(f)
        return {}

    @staticmethod
    def postprocess(response: str) -> str:
        clean_response = ''
        for char in response:
            if char.isalpha() or char.isnumeric() or char == '_':
                clean_response += char
        if clean_response[0].isnumeric():
            clean_response = '_' + clean_response
        return clean_response

    def _get_prompt(self, id: str) -> str:
        with open(f'descriptions/{id}/desc_prompt.txt', 'r') as f:
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

    def _get_model_kwargs(self, messages: List[Dict[str, str]], temperature: float = 0.8, max_tokens: int = 2048) -> Dict[str, Any]:
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
    def get_response(self, prompt) -> str:
        system_prompt = ("You are a helpful AI coding assistant.")

        messages = self._create_messages(prompt, system_prompt)
        kwargs = self._get_model_kwargs(messages)
        
        response = self.create(**kwargs)
        try:
            response = self.get_content(response)[0]
        except Exception as e:
            print(f"{id}: Error: {e}")
            return ""

        return self.postprocess(response)


def get_new_var(old_var, mask_sec_func_base, evaler, vars_all):
    prompt = f"Given the following C/CPP function, create a new variable name for the variable `{old_var}`. \n"
    prompt += "The new variable name should be different from the current variable name. \n"
    prompt += "The new variable should be something a human would name it, which means you should consider how the variable is used in the function. \n"
    prompt += "For example, if the variable `x` tracks the x coordinate location, you could rename it `x_coord`. \n"
    prompt += "If the variable `sum` tracks the cumulative amount owed, you could rename it `cum_amount_owed`. \n"
    prompt += "If the variable `arr` has a customer's address, you could rename it `cust_addr`. \n"
    prompt += "Only return the variable name. \n"
    prompt += "Do not return anything else, such as 'A new variable name could be: ...' or 'The new name is: ...'. \n"
    prompt += f"Here is the C/CPP function: \n"
    prompt += "```\n"
    prompt += mask_sec_func_base
    prompt += "\n```"

    new_var = evaler.get_response(prompt)
    
    while new_var in vars_all:
        prompt += f"\nDo not rename it to {new_var}"
        new_var = evaler.get_response(prompt)
    
    return new_var


# def main():
#     evaler = APIEvaler()
#     with open('ids/final_ids.txt', 'r') as f:
#         ids = f.readlines()

#     for id in tqdm(ids):
#         print(id)
#         response = evaler.get_response(id)
#         evaler.save_cache()
#         with open(f'descriptions/{id}/desc.txt', 'w') as f:
#             f.write(response)
#     evaler.save_cache()

# if __name__ == '__main__':
#     main()