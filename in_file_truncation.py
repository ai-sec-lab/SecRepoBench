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

    system_prompt = SYSTEM_PROMPT
    tokens_system_prompt = general_tokenizer(tokenizer, system_prompt)
    
    prompt = INFILE_PROMPT.format(context="")
    tokens_prompt = general_tokenizer(tokenizer, prompt)

    return len(tokens_system_prompt) + len(tokens_prompt)


def truncate_in_file(id, constant_token_len, tokenizer, max_tokens=10_000):
    # get masked function and its len
    mask_func_desc_perturbed = get_c_cpp_file(f"descriptions/{id}/mask_func_desc_perturbed")
    mask_func_desc_perturbed_len = len(general_tokenizer(tokenizer, mask_func_desc_perturbed))

    # check if there's space for no context case, if not, can't use this id
    no_context_token_len = constant_token_len + mask_func_desc_perturbed_len
    if no_context_token_len > max_tokens:
        print(f"ID {id}: no context length is greater than max_tokens")
        return

    # if the whole mask_desc_perturbed can fit, just use that (no truncation)
    mask_desc_perturbed = get_c_cpp_file(f"descriptions/{id}/mask_desc_perturbed")
    mask_desc_perturbed_len = len(general_tokenizer(tokenizer, mask_desc_perturbed))
    whole_context_len = constant_token_len + mask_desc_perturbed_len
    if whole_context_len <= max_tokens:
        return mask_desc_perturbed

    # Now we truncate the in-file context
    truncation_notice = "\n\n// --- CODE TRUNCATED HERE ---\n\n"
    truncation_notice_len = len(general_tokenizer(tokenizer, truncation_notice))

    context_token_limit = max_tokens - constant_token_len - mask_func_desc_perturbed_len - truncation_notice_len

    if mask_desc_perturbed.count(mask_func_desc_perturbed) != 1:
        print(f"ID {id}: mask_func_desc_perturbed not in mask_desc_perturbed")
        return
    text_before, text_after = mask_desc_perturbed.split(mask_func_desc_perturbed)

    text_before_tokens = general_tokenizer(tokenizer, text_before)
    text_after_tokens = general_tokenizer(tokenizer, text_after)

    text_before_token_len = len(text_before_tokens)
    text_after_token_len = len(text_after_tokens)

    if text_before_token_len > context_token_limit:
        # need to truncate the beginning
        keep_tokens = text_before_tokens[:context_token_limit]
        keep_text = tokenizer.decode(keep_tokens)
        context = keep_text + truncation_notice + mask_func_desc_perturbed
        return context
    else:
        # need to truncate the end
        context_token_limit -= text_before_token_len
        keep_tokens = text_after_tokens[:context_token_limit]
        keep_text = tokenizer.decode(keep_tokens)
        context = text_before + mask_func_desc_perturbed + keep_text + truncation_notice
        return context


def main():
    tokenizer = AutoTokenizer.from_pretrained("deepseek-ai/deepseek-coder-1.3b-instruct", trust_remote_code=True)

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    max_tokens = 10_000

    constant_token_len = get_constant_prompt_token_len(tokenizer)

    for id in ids:
        context = truncate_in_file(id, constant_token_len, tokenizer, max_tokens=max_tokens)

        # sanity check
        full = SYSTEM_PROMPT + INFILE_PROMPT.format(context=context)
        full_tokens_len = len(general_tokenizer(tokenizer, full))
        if full_tokens_len > max_tokens:
            print(f"ID {id}: does not pass sanity check")
        
        if "// <MASK>" not in full:
            print(f"ID {id}: does not pass sanity check")

        if context is not None:
            with open(f"descriptions/{id}/in-file-truncated.txt", "w") as f:
                f.write(context)


if __name__ == '__main__':
    main()
