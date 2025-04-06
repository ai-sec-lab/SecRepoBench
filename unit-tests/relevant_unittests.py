import json
import re
from collections import defaultdict

from tqdm import tqdm
from projects import *
test_before_projects = ['ffmpeg', 'file', 'c-blosc2', 'fluent-bit', 'assimp', 'php-src', 'libxml2', 'imagemagick', 'mruby', 'wireshark','libarchive','openexr', 'libredwg', 'libxslt']
no_colon_projects = ['harfbuzz', 'libplist', 'yara']

<<<<<<< HEAD:unit-tests/relevant_unittests.py
def remove_ansi(text):
    ansi_escape = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape.sub('', text)
=======
def remove_repeated_blocks(text):
    repeated_block = r'(This is a test for CodeGuard\+\n)(\1)+'
    cleaned_text = re.sub(repeated_block, r'\1', text)
    return cleaned_text
>>>>>>> 5634a6146b737d908b13dc3cdaaeec2af1d090b4:relevant_unittests.py

def get_relevant_unittests(target_project, stdout):
    test_before = target_project in test_before_projects
    no_colon = target_project in no_colon_projects
    # Pattern to match unit test results
    pattern = unittest_patterns[target_project]
    if no_colon:
        pattern = r"\n(?P<status>[A-Z]+) (?P<name>.*) \("
    if target_project == 'php-src':
        pattern = r"[\r\n](?P<status>PASS|FAIL|SKIP).*?\[(?P<name>[^\]]+)\]"
        stdout = remove_ansi(stdout)
    elif target_project == 'c-blosc2' or target_project == 'libdwarf':
        pattern = r"Test: (?P<name>.*)\n"
    elif target_project == 'fluent-bit':
        pattern = r"Test (?P<name>.*)\.\.\."
<<<<<<< HEAD:unit-tests/relevant_unittests.py
    elif target_project == 'imagemagick':
=======
    elif target_project == 'php-src':
>>>>>>> 5634a6146b737d908b13dc3cdaaeec2af1d090b4:relevant_unittests.py
        pattern = r"Test Name: (?P<name>.*)\n"
        matches = list(re.finditer(pattern, stdout))
        relevant_unittests = []
        for match in matches:
            base_name = re.escape(match.group("name"))
            full_pattern = rf"\[(?P<name>[^\]]*{base_name}\.phpt)\]"
            full_name = re.findall(full_pattern, stdout)[0]
            relevant_unittests.append(full_name)
        return relevant_unittests
    elif target_project == 'libxml2':
        pattern = r"## (?P<name>.*)\n"
    elif target_project == 'mruby':
        patterns = [
            r"(?P<name>.*?) : This is a test for CodeGuard\+\n(?P<status>\.|F)\n",
            r"(?P<name>.*?) : (?P<status>\.|F)\nThis is a test for CodeGuard\+"
        ]
        relevant_unittests = []
        for pattern in patterns:
            for match in re.finditer(pattern, stdout):
                name = match.group("name")
                relevant_unittests.append(name)
        return relevant_unittests
    elif target_project == 'matio':
        pattern = r'(?P<number>\d+)\. .*?testing (?P<name>.*?) \.{3}.*?\+This is a test for CodeGuard\+'
    elif target_project == 'htslib':
        pattern = r"Testing (?P<name>.*?)\.\.\.[\s\S]*?cd "
        relevant_unittests = []
        for match in re.finditer(pattern, stdout):
            if 'This is a test for CodeGuard' in match.group():
                name = match.group("name")
                relevant_unittests.append(name)
        return relevant_unittests
    elif target_project == 'openexr':
        pattern = r'(?m)^(?:\d+/\d+\s+)?Test:\s+(?P<name>[\w\.]+)'
    elif target_project == 'pcapplusplus':
        pattern = r'(?P<name>\w+)\s+: [A-Z]+\n'
    elif target_project == 'wireshark':
        # Technically, this will not work for all cases but we only have one case (test is before)
        pattern = r'\n(?P<name>test_sharkd_(\w|_)+) \('
    elif target_project == 'libarchive':
        pattern = r'(?m)^(?:\d+/\d+\s+)?Test:\s+(?P<name>[\w\.\+]+)'
    elif target_project=="flac":
        block = r'(This is a test for CodeGuard\+)'
        cleaned_text = re.sub(block, r'This is a test for CodeGuard', stdout)  # plus sign ruins regex
        pattern = r"\+\+\+ .*?test: (?P<name>.*?)\n[^\+]*?(?P<status>PASSED)!"
        relevant_unittests = []
        for match in re.finditer(pattern, cleaned_text):
            if 'This is a test for CodeGuard' in match.group():
                name = match.group("name")
                relevant_unittests.append(name)
        return relevant_unittests
    elif target_project == 'libredwg':
        pattern = r'(?P<status>ok|not ok)\s+(?P<name>\d+.*?)\nThis is a test for CodeGuard\+'
        relevant_unittests = []
        for match in re.finditer(pattern, stdout):
            name = match.group("name")
            relevant_unittests.append(name)
        return relevant_unittests
    elif target_project == 'libxslt':
        pattern = r'## Running (?P<name>.*) tests'
    elif target_project == 'libsndfile':
        relevant_unittests = []
        patterns = [
            r" {4}(?P<name>[\w\(\) \/]+ +: .*)\s+\.+\s+This is a test for CodeGuard\+\n(?P<status>\w+)\n",
            r" {4}(?P<name>[\w\(\) \/]+ +: .*This is a test for CodeGuard\+\n.*)\s+\.+\s+(?P<status>\w+)\n"
        ]
        for pattern in patterns:
            matches = list(re.finditer(pattern, stdout))
            for match in matches:
                name = match.group("name").replace(r"This is a test for CodeGuard+\n", "")
                relevant_unittests.append(name)
        return relevant_unittests

    elif target_project == 'wolfssl':
        pattern = r'\n(?P<name>\w+)\s+test'
    elif target_project == 'hunspell':
        pattern = r'\n(?P<status>[A-Z]+) (?P<name>.*\.dic)'

    # reduce the redundant print statements
    stdout = remove_repeated_blocks(stdout)

    # Track tests that called the function
    relevant_unittests = []

    if target_project == 'matio':
        matches = list(re.finditer(pattern, stdout, re.DOTALL))
        for match in matches:
            test = f'{match.group("number")}: {match.group("name")}'
            relevant_unittests.append(test)
        return relevant_unittests

    # Find all matches with their positions
    matches = list(re.finditer(pattern, stdout))
    
    # Search for occurrences of "This is a test for CodeGuard+"
    function_call_positions = [m.start() for m in re.finditer(r"This is a test for CodeGuard\+", stdout)]

    if not function_call_positions:
        return relevant_unittests  # No function calls detected

    last_test_pos = 0
    last_test_name = None

    # Iterate through each unit test match
    # If test_before is true, we consider the first unit after the function call, rather the last one before
    for match in matches:
        test_name = match.group("name")
        test_pos = match.start()  # Position of test match in text
        if target_project == 'mruby' and 'assert' in test_name:
            continue

        # Check if any function call happened after last test but before this one
        if any(last_test_pos < pos < test_pos for pos in function_call_positions):
            if test_before:
                if last_test_name:
                    relevant_unittests.append(last_test_name)
            else:
                relevant_unittests.append(test_name)

        # Update last test position and name
        last_test_pos = test_pos
        last_test_name = test_name

    # check after the last match
    if test_before and any(last_test_pos < pos for pos in function_call_positions):
        relevant_unittests.append(last_test_name)
    
    if target_project == 'wireshark':
        relevant_unittests.pop()

    return list(set(relevant_unittests))


def main():
    # Say that file's test are used in test.c
    target_projects = ['lcms', 'file', 'ffmpeg', 'libxml2', 'imagemagick', 'harfbuzz', 'yara', 'flac', 'libxslt', 'htslib', 'ndpi', 'mruby', 'php-src', 'c-blosc2', 'assimp', 'libsndfile', 'wolfssl', 'fluent-bit', 'matio', 'wireshark', 'gpac', 'libarchive', 'libplist', 'libdwarf', 'openexr', 'hunspell', 'libredwg', 'pcapplusplus']

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]
    
    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)

    ids_by_proj = defaultdict(list)
    for id in ids:
        project = cases[id]['project_name']
        ids_by_proj[project].append(id)

    results = {}
    for target_project in target_projects:
        print(f"Processing {target_project}")
        ids_proj = ids_by_proj[target_project]

        for id in tqdm(ids_proj):
            with open(f'/data/oss-fuzz-bench/output/{id}/unittest_sec_print/stdout.txt', 'r') as f:
                stdout = f.read()

            relevant_unittests = get_relevant_unittests(target_project, stdout)
            results[id] = {"relevant_unittests": relevant_unittests}

    with open('relevant_unittests.json', 'w') as f:
        json.dump(results, f, indent=4)


if __name__ == "__main__":
    main()
