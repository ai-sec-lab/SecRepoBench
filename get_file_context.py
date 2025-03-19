import os
from tqdm import tqdm
import json
import tree_sitter_c as tsc
import tree_sitter_cpp as tscpp
from tree_sitter import Language, Parser

from constants import *
from cwe_map import *


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


def get_file_context(input_path):
    content = get_c_cpp_file(input_path + '/mask_desc_perturbed')

    # # Tokenize the content
    # tokenizer = MistralTokenizer.v3()
    # completion_request = ChatCompletionRequest(messages=[UserMessage(content=content)])    
    # tokens = tokenizer.encode_chat_completion(completion_request).tokens
    # token_count = len(tokens)

    # If added in file context is less than or equal to limit, save directly
    # context_len = token_count - context_len_limit["mask_func_desc_perturbed_token_len"]
    # if context_len <= context_len_limit["context_len_limit"]:
    with open(os.path.join(input_path, 'in-file.txt'), 'w') as output_file:
        output_file.write(content)
    # else:
    #     print(f'context_len for file {input_path} is {context_len} but has a limit of {context_len_limit["context_len_limit"]}, truncating the context len')
    #     # if this is just a few -- do manually ! 132 cases, too many for manual
    #     truncated_content = truncate_content(content)
    #     with open(os.path.join(input_path, 'in-file.txt'), 'w') as output_file:
    #         output_file.write(truncated_content)

def main():
    # use get_context_len_limits.py to see which files need to be manually truncated.
    # once everything is under the token limit, you can run this file, which 
    # just copies the mask_desc_perturbed to in-file

    input_dir = 'descriptions'

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    # with open("context_len_limits.json", 'r') as f:
    #     context_len_limits = json.load(f)

    for id in ids:
        # context_len_limit = context_len_limits[id]

        input_path = os.path.join(input_dir, id)
        if os.path.exists(os.path.join(input_path, 'in-file.txt')):
            os.remove(os.path.join(input_path, 'in-file.txt'))
        get_file_context(input_path)


if __name__ == '__main__':
    main()