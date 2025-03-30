import os
import re


def remove_repeated_blocks(text):
    repeated_block = r'(This is a test for CodeGuard\+\n)(\1)+'
    cleaned_text = re.sub(repeated_block, r'\1', text)
    return cleaned_text

def remove_ansi(text):
    ansi_escape = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape.sub('', text)

if __name__ == "__main__":

    root_dir = "/data/oss-fuzz-bench/output"
    files_and_directories = [d for d in os.listdir(root_dir)]

    for directory in files_and_directories:
        stdout_path = os.path.join(root_dir, directory, "unittest_sec_print", "stdout.txt")
        if not os.path.exists(stdout_path):
            continue
        with open(stdout_path, "rb") as f:
            stdout = f.read().decode('utf-8', errors='ignore')

        clean_stdout = remove_ansi(stdout)

        with open(stdout_path, "w") as f:
            f.write(clean_stdout)
