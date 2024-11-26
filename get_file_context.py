import tiktoken
import os
from tqdm import tqdm
import json

def find_special_symbol_tokens(enc, tokens, special_symbol):
    # Find the index where the special symbol starts
    for i in range(len(tokens)):
        if special_symbol in enc.decode(tokens[i:]):
            if special_symbol not in enc.decode(tokens[i+1:]):
                return i, i+len(enc.encode(special_symbol))-1

    # If the symbol is not found, return None
    return None, None

def get_file_context(input_path):
    with open(os.path.join(input_path, 'mask_perturbed.txt'), 'r') as file:
        content = file.read()

    with open(os.path.join(input_path, 'mask_func_perturbed.txt'), 'r') as output_file:
        func = output_file.read()

    # Tokenize the content
    enc = tiktoken.encoding_for_model("gpt-4o")
    tokens = enc.encode(content)
    token_count = len(tokens)

    # If token count is less than or equal to 20k, save directly
    if token_count <= 20000:
        with open(os.path.join(input_path, 'new-in-file.txt'), 'w') as output_file:
            output_file.write(content)
    else:
        print(f'Token count for file {input_path} is {token_count}, truncating the content to 15k tokens')
        reserved_token_num = len(enc.encode(func))
        truncated_content = enc.decode(tokens[:20000-reserved_token_num])
        with open(os.path.join(input_path, 'new-in-file.txt'), 'w') as output_file:
            output_file.write(truncated_content)

def main():
    input_dir = 'descriptions'
    with open('/home/yanjun/arvo-oss-fuzz-bench/ids/final_ids.txt', 'r') as f:
        ids = f.readlines()
        ids = sorted([int(id.strip()) for id in ids])
    for id in tqdm(ids):
        id = str(id)
        input_path = os.path.join(input_dir, id)
        if os.path.exists(os.path.join(input_path, 'new-in-file.txt')):
            os.remove(os.path.join(input_path, 'new-in-file.txt'))
        get_file_context(input_path)

if __name__ == '__main__':
    main()