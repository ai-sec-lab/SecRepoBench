from collections import defaultdict
import json
import subprocess
import re
import csv
import random
from tqdm import tqdm
import requests

random.seed(43)

with open('../filter_logs/cases.json', 'r') as f:
    cases = json.load(f)

with open('../ids.txt', 'r') as f:
    ids = f.read().splitlines()

with open('relevant_unittests.json', 'r') as f:
    relevant_unittests = json.load(f)
    relevant_unittests = {sample_id: relevant_unittests[sample_id] for sample_id in ids}
    one_test_samples = [sample_id for sample_id in relevant_unittests if len(relevant_unittests[sample_id]['relevant_unittests']) == 1]
    # choose 2 random samples for each project
    seen_set = defaultdict(int)
    random_sample = []
    all_samples = [sample_id for sample_id in relevant_unittests]
    random.shuffle(all_samples)
    for sample_id in all_samples:
        project_name = cases[sample_id]['project_name']
        if seen_set[project_name] < 2:
            random_sample.append(sample_id)
            seen_set[project_name] += 1
    # merge one_test_samples and random_sample
    relevant_unittests = {sample_id: relevant_unittests[sample_id] for sample_id in random_sample}


def execute_command(command, sample_id):
    return subprocess.run(['docker', 'run', '--rm', f'n132/arvo:{sample_id}-fix', 'sh', '-c', command], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()

def get_github_base(sample_id):
    with open(f'../ARVO-Meta/meta/{sample_id}.json', 'r') as f:
        meta = json.load(f)
    return meta['repo_addr']

def get_comment(sample_id):
    project_name = cases[sample_id]['project_name']
    if project_name == 'ffmpeg':
        return f'Actual testing code located in unknown file'
    elif project_name == 'hunspell':
        return f'Actual testing code located in tests/test.sh'
    elif project_name == 'file':
        return f'Actual testing code located in test.c'
    elif project_name == 'libdwarf':
        return f'Arguments to the test file are located in test/CMakeLists.txt'
    elif project_name == 'ndpi':
        return f'Actual testing code located in tests/do.sh'
    elif project_name == 'libxslt':
        return f'Links to a directory of relevant tests'
    else:
        return ""

def get_file_and_line_no(test_name, sample_id):
    FIND_COMMAND = f'find . -wholename "*{test_name}*"'
    line_number = 0
    if project_name == 'ffmpeg':
        # Actual testing code located in unknown file
        command = FIND_COMMAND
        output = execute_command(command, sample_id)
        file_name = output.split('\n')[0]
        if output == '':
            command = f'grep -rn "fate-{test_name}: C" .'
            output = execute_command(command, sample_id)
            file_name = output.split(':')[0]
            line_number = output.split(':')[1]
    elif project_name == 'hunspell':
        # Actual testing code located in test.sh
        file_name = f'tests/{test_name}.dic'
    elif project_name == 'c-blosc2':
        if test_name.endswith('0_1') or test_name.endswith('1_1') or test_name.endswith('2_1'):
            test_name = re.sub(r'_?\d+', '', test_name)
        if test_name.startswith('test_example'):
            test_name = test_name[13:]
            file_name = f'examples/{test_name}.c'
        elif test_name.startswith('test_compat'):
            test_name = test_name[11:]
            file_name = f'compat/{test_name}'
        elif 'lizard' in test_name:
            # can't get line number
            file_name = 'bench/CMakelists.txt'
        else:
            file_name = f"tests/{test_name}.c"
    elif project_name == 'gpac':
        command = f'grep -rI "{test_name}" testsuite'
        output = execute_command(command, sample_id)
        if len(output) == 0:
            new_test_name = test_name.split('-')[0] + '-\\\\$'
            command = f'grep -rI "{new_test_name}" testsuite'
            output = execute_command(command, sample_id)
        file_name = output.split(':')[0][10:]
        if test_name.startswith('dash_exotic'):
            file_name = 'scripts/dash_exotic.sh'
        elif test_name.startswith('counter_cues'):
            file_name = 'scripts/dash_cues.sh'
        elif test_name.startswith('raw-aud'):
            file_name = 'scripts/raw-audio.sh'
        elif test_name.startswith('raw-vid'):
            file_name = 'scripts/raw-video.sh'
    elif project_name == 'lcms':
        command = f'grep -r "\\"{test_name}\\"" .'
        output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        function_name = output.split(',')[1][:-2].strip()
        command = f'grep -n "{function_name}(" {file_name}'
        output = execute_command(command, sample_id)
        if len(output) == 0:
            command = f'grep -n "{function_name}(" testbed/testplugin.c'
            file_name = 'testbed/testplugin.c'
            output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'assimp':
        file_base, new_test_name = test_name.split('.')
        command = f'grep --include=*.cpp  -r "{file_base}" .'
        file_name = execute_command(command, sample_id).split(':')[0]
        command = f'grep -n "{new_test_name}" {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'harfbuzz' or project_name == 'imagemagick':
        command = f'find . -wholename "*{test_name}.*"'
        file_name = execute_command(command, sample_id).split('\n')[0]
        if file_name == '':
            command = FIND_COMMAND
            file_name = execute_command(command, sample_id).split('\n')[0]
    elif project_name == 'fluent-bit':
        command = f'grep -r -E "[\\\'\\\"]{test_name}[\\\'\\\"]" .'
        output = execute_command(command, sample_id)
        function_name = output.split(',')[1].lstrip()[:-1]
        command = f'grep -rn "{function_name}(" .'
        output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        line_number = output.split(':')[1]
    elif project_name == 'file':
        # Actual testing code located in test.c
        file_name = test_name[1:]
    elif project_name == 'libplist':
        file_name = f'test/{test_name}'
    elif project_name == 'libdwarf':
        # Testing done by the same file, different arguments
        # add_test(NAME selfdwarfdumpelf COMMAND ${elfshdir}/dwarfdumptest.py Elf cmake
        file_name = 'test/CMakeLists.txt'
        command = f'grep -n "add_test(NAME {test_name}" {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'yara':
        file_name = f'tests/{test_name}.c'
    elif project_name == 'libxml2':
        # need more reliable method
        if 'fuzzer' in test_name:
            file_name = 'fuzz/testFuzzer.c'
        elif 'examples regression' in test_name:
            file_name = 'doc/examples/Makefile.am'
            line_number = 96
        else:
            command = f'grep -r "{test_name}" . -A 1 --include=*.c ' 
            output = execute_command(command, sample_id).split('\n')[1]
            file_name = output.split('-')[0]
            function_name = output.split('-')[1].split(',')[0].strip()
            command = f'grep -n "{function_name}(" {file_name}'
            output = execute_command(command, sample_id)
            line_number = output.split(':')[0]
    elif project_name == 'php-src':
        file_name = test_name
    elif project_name == 'ndpi':
        file_name = f'tests/pcap/{test_name}'
    elif project_name == 'mruby':
        if test_name.startswith('Direct superclass of'):
            command = f'grep --include=*.rb -r "Direct superclass of" .'
        else:
            test_name = re.escape(test_name.split(" [1")[0]).replace('`', '\\`').replace('"', '\\"')
            command = f'grep --include=*.rb -r -E "[\\\'\\\"]{test_name}[\\\'\\\"]" .'
        output = execute_command(command, sample_id)
        if output == '':
            test_name = test_name.split('#')[1]
            command = f'grep -r "%w\\[{test_name}" .'
            output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        command = f'grep -n "{test_name}" {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
        if "#" not in test_name:
            line_number = str(int(line_number) + 1)
    elif project_name == 'htslib':
        file_name = 'test/test-regidx.c'
    elif project_name == 'matio':
        split_name = test_name.split(':')
        file_name = f'test/tests/{split_name[0]}'
        line_number = split_name[1]
    elif project_name == 'openexr':
        test_name = test_name.split('.')[1]
        command = f'grep --include=*.cpp -r "{test_name} (" .'
        output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        command = f'grep -n "{test_name} (" {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'libarchive':
        test_name = test_name[11:]
        command = f'grep --include=*.c -r "{test_name}" .'
        output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        command = f'grep -n "{test_name}" {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'libredwg':
        file_name = f'test/unit-testing/{test_name}.c'
    elif project_name == 'pcapplusplus':
        command = f'grep -r "{test_name}" Tests/Packet++Test/Tests/'
        output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        command = f'grep -n "{test_name}" {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'libxslt':
        test_name = '/'.join(test_name.split(' '))
        file_name = f'tests/{test_name}'
    elif project_name == 'libsndfile':
        overall_test, sub_test = test_name.split(':')
        overall_test = overall_test.strip()
        sub_test = sub_test.strip().replace('.', '\\.')
        if '_' in sub_test:
            sub_test = '_'.join(sub_test.split('_')[1:])
        command = f'grep -r -E \'{overall_test}\\s+\\("{sub_test}\' .'
        output = execute_command(command, sample_id)
        file_name = output.split(':')[0]
        command = f'grep -n -E \'{overall_test}\\s+\\("{sub_test}\' {file_name}'
        output = execute_command(command, sample_id)
        line_number = output.split(':')[0]
    elif project_name == 'wireshark':
        # temporary fix, only one sample
        file_name = "test/suite_sharkd.py"
        line_number = 453
    elif project_name == 'wolfssl':
        file_name = "wolfcrypt/test/test.c"
        line_number = 21277
    elif project_name == 'flac':
        if 'StreamEncoder' in test_name:
            file_name = 'src/test_libFLAC++/encoders.cpp'
            line_number = 196
        elif 'StreamDecoder' in test_name:
            file_name = 'src/test_libFLAC++/decoders.cpp'
            line_number = 440
        else:
            file_name = 'src/test_libFLAC++/metadata_object.cpp'
            line_number = 2069
    
    if file_name.startswith('./'):
        file_name = file_name[2:]
    if file_name.startswith(f'{project_name}/'):
        file_name = file_name[(len(project_name))+1:]

    return file_name, line_number

def find_github_url(test_name, sample_id):
    project_name = cases[sample_id]['project_name']
    commit_hash = cases[sample_id]['fixing_commit']
    github_base = get_github_base(sample_id)
    file_name, line_number = get_file_and_line_no(test_name, sample_id)
    if file_name == '':
        print("Failed to find file name for ", test_name)
        exit(1)
        
    # if file_name starts with project_name, remove it
    if project_name == 'ffmpeg':
        return f'https://git.ffmpeg.org/gitweb/ffmpeg.git/blob/{commit_hash}:/{file_name}#L{line_number}'
    elif project_name == 'matio':
        return f'https://github.com/tbeu/matio/blob/{commit_hash}/{file_name}#L{line_number}'
    else:
        if github_base.endswith('.git'):
            github_base = github_base[:-4]
        if project_name == 'gpac':
            commit_hash = execute_command('cd testsuite && git rev-parse HEAD', sample_id)
            github_base = github_base[:-4] + 'testsuite'
        return f'{github_base}/blob/{commit_hash}/{file_name}#L{line_number}'

all_samples = []
for sample_id in relevant_unittests:
    tests = relevant_unittests[sample_id]['relevant_unittests']
    random.shuffle(tests)
    if len(tests) > 5:
        tests = tests[:5]
    
    project_name = cases[sample_id]['project_name']
    print(f'Processing {sample_id} {project_name}')
    
    #  docker run --rm -it n132/arvo:13736-vul
    test_urls = []
    for test_name in tqdm(tests):
        url = find_github_url(test_name, sample_id)
        test_urls.append(url)
        # check if the url is valid
        # response = requests.get(url)
        # if response.status_code != 200:
        #     print(f'Invalid url: {url}')
        #     exit(1)

    
    all_samples.append([sample_id, project_name, get_comment(sample_id), *test_urls])
    
# sort all samples by project name
all_samples.sort(key=lambda x: x[1])
with open('unit_test_urls.csv', 'w') as f:
    writer = csv.writer(f)
    for sample in all_samples:
        writer.writerow(sample)