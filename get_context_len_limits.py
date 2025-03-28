import tiktoken
import os
from tqdm import tqdm
import json

from constants import *
from cwe_map import *

from mistral_common.tokens.tokenizers.mistral import MistralTokenizer
from transformers import AutoTokenizer


def gpt_4o_tokenize(text):
    # Tokenize the content
    tokenizer = tiktoken.encoding_for_model("gpt-4o")
    tokens = tokenizer.encode(text)
    return tokens


def general_tokenizer(tokenizer, text):
    tokens = tokenizer.encode(text)
    # tokens = tokenizer.instruct_tokenizer.tokenizer.encode(text, bos=True, eos=True)
    return tokens


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


def get_constant_prompt_token_len(tokenizer):

    system_prompts = [SYSTEM_PROMPT, SEC_GENERIC_PROMPT]
    prompts = [INFILE_PROMPT.format(context=""),
               CROSS_FILE_PROMPT.format(context1="", context2=""),
               FUNC_PROMPT.format(context="")]

    # add cwe id, desc to SEC_SPECIFIC_PROMPT
    for CWE_name, CWE_ID in crash_type_to_cwe.items():
        CWE_description = cwe_id_to_desc[CWE_ID]
        system_prompt = SEC_SPECIFIC_PROMPT.format(CWE_ID=CWE_ID, CWE_description=CWE_description)
        system_prompts.append(system_prompt)

    system_prompt_token_lens = []
    for system_prompt in system_prompts:
        tokens = general_tokenizer(tokenizer, system_prompt)
        system_prompt_token_lens.append(len(tokens))

    prompts_token_lens = []
    for prompt in prompts:
        tokens = general_tokenizer(tokenizer, prompt)
        prompts_token_lens.append(len(tokens))
    
    max_system_prompt_token_lens = max(system_prompt_token_lens)
    max_prompts_token_lens = max(prompts_token_lens)
    # print(f"Max system prompt: {max_system_prompt_token_lens}")
    # print(f"Max prompt: {max_prompts_token_lens}")

    return max_system_prompt_token_lens + max_prompts_token_lens


def get_context_len_limit(id, max_prompt_len, context_len_limits, tokenizer, name):
    # get max token len of sec, vul code blocks
    sec_block = get_c_cpp_file(f"descriptions/{id}/sec_code_block_perturbed")
    sec_tokens = general_tokenizer(tokenizer, sec_block)

    vul_block = get_c_cpp_file(f"descriptions/{id}/vul_code_block_perturbed")
    vul_tokens = general_tokenizer(tokenizer, vul_block)

    max_block_token_len = max(len(sec_tokens), len(vul_tokens))

    # get token len of masked_func + desc
    mask_func_desc_perturbed = get_c_cpp_file(f"descriptions/{id}/mask_func_desc_perturbed")
    mask_func_desc_perturbed_token_len = len(general_tokenizer(tokenizer, mask_func_desc_perturbed))

    # get context len limit, smallest context window of 128k
    context_len_limit = 128_000 - max_prompt_len - 2 * max_block_token_len - mask_func_desc_perturbed_token_len

    # # get in file context, which is in file tokens minus target func + desc tokens
    # mask_desc_perturbed = get_c_cpp_file(f"descriptions/{id}/mask_desc_perturbed")
    # mask_desc_perturbed_tokens = general_tokenizer(tokenizer, mask_desc_perturbed)
    # mask_desc_perturbed_token_len = len(mask_desc_perturbed_tokens) - mask_func_desc_perturbed_token_len

    # if mask_desc_perturbed_token_len > context_len_limit:
    #     print(f"ID {id} tokenizer {name}: context len {mask_desc_perturbed_token_len}, limit {context_len_limit}")

    # get cross file context
    with open(f'descriptions/{id}/cross-file.txt', 'r') as f:
        cross_file = f.read()
    cross_file_tokens = general_tokenizer(tokenizer, cross_file)
    cross_file_token_len = len(cross_file_tokens)

    if cross_file_token_len > context_len_limit:
        print(f"ID {id} tokenizer {name}: context len {cross_file_token_len}, limit {context_len_limit}")

    context_len_limits[id] = {
        "max_prompt_len": max_prompt_len,
        "max_block_token_len": max_block_token_len,
        "max_block_token_with_SF_len": 2 * max_block_token_len,
        "mask_func_desc_perturbed_token_len": mask_func_desc_perturbed_token_len,
        "context_len_limit": context_len_limit
    }


def main():
    # tokenize_func = mistral_tokenize
    tokenize_funcs_names = [
        # (MistralTokenizer.from_file("tekken.json"), 'mistral'),
        # (tiktoken.encoding_for_model("gpt-4o"), 'gpt4o'),
        # (AutoTokenizer.from_pretrained("deepseek-ai/DeepSeek-Coder-V2-Lite-Instruct", trust_remote_code=True), 'deepseek'),
        # (AutoTokenizer.from_pretrained("meta-llama/Llama-3.1-8B-Instruct", trust_remote_code=True), 'llama'),
        AutoTokenizer.from_pretrained("deepseek-ai/deepseek-coder-1.3b-instruct", trust_remote_code=True)
    ]

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    context_len_limits = {}
    for id in ids:
        # print(id)
        for tokenizer, name in tokenize_funcs_names:
            # print(name)
            max_prompt_len = get_constant_prompt_token_len(tokenizer)
            get_context_len_limit(id, max_prompt_len, context_len_limits, tokenizer, name)
    
    context_len_limits = dict(sorted(context_len_limits.items(), key=lambda item: item[1]["context_len_limit"]))
    with open("context_len_limits.json", 'w') as f:
        json.dump(context_len_limits, f, indent=4)


if __name__ == '__main__':
    main()