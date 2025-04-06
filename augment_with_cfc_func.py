# Copyright Amazon.com, Inc. or its affiliates. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import json
import time
import glob
import subprocess
import argparse
import multiprocessing as mp
from tqdm import tqdm
from functools import partial
from rerank_utils import lexical_ranking, SemanticReranking
from utils import str2bool, file_distance, tokenize_nltk
from tree_sitter import Language, Parser
from utils import *

CHUNK_SIZE = 50
SLIDING_WINDOW_SIZE = 50  # non-overlapping chunks if SLIDING_WINDOW_SIZE=CHUNK_SIZE
QUERY_LENGTH = 10  # last N lines from prompt will be query

repository_root = "/PATH/TO/REPOS"  # get the data from authors

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

input_files = {
    "python": "../data/crosscodeeval_data/python/line_completion.jsonl",
    "java": "../data/crosscodeeval_data/java/line_completion.jsonl",
    "typescript": "../data/crosscodeeval_data/typescript/line_completion.jsonl",
    "csharp": "../data/crosscodeeval_data/csharp/line_completion.jsonl"
}

file_ext = {
    "python": "py", 
    "java": "java", 
    "typescript": ["ts", "tsx"], 
    "csharp": "cs",
    "c": ["c", "h", "cpp", "cc", "hh", "hpp", "cxx"],
    }


def get_crossfile_context_from_chunks(
        args,
        query,
        code_chunks,
        code_chunk_ids,
        semantic_ranker
):
    assert len(code_chunks) != 0
    candidate_code_chunks = code_chunks[:args.maximum_chunk_to_rerank]
    candidate_code_chunk_ids = code_chunk_ids[:args.maximum_chunk_to_rerank]

    ranking_scores = None
    meta_data = {}

    if args.rerank:

        meta_data["query"] = query
        start = time.time()

        if args.ranking_fn == "cosine_sim":
            gpu_id = int(mp.current_process().name.split('-')[-1]) - 1
            candidate_code_chunks, candidate_code_chunk_ids, ranking_scores = semantic_ranker.rerank(
                query,
                candidate_code_chunks,
                candidate_code_chunk_ids,
                gpu_id,
                score_threshold=None
            )
        else:
            candidate_code_chunks, candidate_code_chunk_ids, ranking_scores = lexical_ranking(
                query,
                candidate_code_chunks,
                args.ranking_fn,
                candidate_code_chunk_ids,
                score_threshold=None
            )

        meta_data["latency"] = time.time() - start
        meta_data["num_candidates"] = len(candidate_code_chunks)

    top_k = min(args.maximum_cross_file_chunk, len(candidate_code_chunk_ids))
    if top_k == 0:
        return [], meta_data

    selected_chunks = []
    selected_chunks_filename = []
    selected_chunks_scores = []

    if args.use_next_chunk_as_cfc:
        # prepare an id2idx map
        assert len(candidate_code_chunks) == len(candidate_code_chunk_ids)
        id2idx = dict()
        for j, cci in enumerate(code_chunk_ids):
            id2idx[cci] = j

        total_added = 0
        for cidx, _id in enumerate(candidate_code_chunk_ids):
            fname, c_id = _id.rsplit("|", 1)
            next_id = f"{fname}|{int(c_id) + 1}"
            if next_id not in id2idx:
                to_add = code_chunks[id2idx[_id]]
            else:
                to_add = code_chunks[id2idx[next_id]]

            if to_add not in selected_chunks:
                selected_chunks.append(to_add)
                selected_chunks_filename.append(fname)
                if args.rerank:
                    selected_chunks_scores.append(ranking_scores[cidx])
                total_added += 1
                if total_added == top_k:
                    break
    else:
        selected_chunks = candidate_code_chunks[:top_k]
        selected_chunks_filename = [_id.rsplit("|", 1)[0] for _id in candidate_code_chunk_ids[:top_k]]
        if args.rerank:
            selected_chunks_scores = ranking_scores[:top_k]

    cross_file_context = []
    for idx in range(len(selected_chunks)):
        cross_file_context.append({
            "retrieved_chunk": selected_chunks[idx],
            "filename": selected_chunks_filename[idx],
            "score": selected_chunks_scores[idx] if args.rerank else None
        })

    line_start_sym = "#" if args.language == "python" else "//"
    cfc_text = f"{line_start_sym} Here are some relevant code fragments from other files of the repo:\n\n"
    for sc, scf in zip(selected_chunks, selected_chunks_filename):
        cfc_text += f"{line_start_sym} the below code fragment can be found in:\n{line_start_sym} {scf}" + "\n"
        cfc_text += sc.strip('\n') + "\n\n"

    return cross_file_context, cfc_text, meta_data


def read_project_files(repo_path, lang):
    # root_dir needs a trailing slash (i.e. /root/dir/)
    project_context = {}
    root_dir = repo_path
    if not os.path.isdir(root_dir):
        print(f"Repository not found: {root_dir}")
        return project_context

    extensions = file_ext[lang] if isinstance(file_ext[lang], list) else [file_ext[lang]]
    src_files = []
    for ext in extensions:
        src_files += glob.glob(os.path.join(root_dir, f'**/*.{ext}'), recursive=True)

    if len(src_files) == 0:
        return project_context

    for filename in src_files:
        if os.path.exists(filename):  # weird but some files cannot be opened to read
            if os.path.isfile(filename):
                try:
                    with open(filename, "r") as file:
                        file_content = file.read()
                except:
                    with open(filename, "rb") as file:
                        file_content = file.read().decode(errors='replace')

                fileid = os.path.relpath(filename, root_dir)
                project_context[fileid] = file_content
            else:
                pass
                print(f"File is not a regular file: {filename}")
        else:
            pass
            print(f"File not found: {filename}")

    return project_context


def find_files_within_distance_k(current_file_path, filelist, k):
    list_of_modules = []
    module_weight = []
    for filepath in filelist:
        if filepath != current_file_path:
            dist = file_distance(filepath, current_file_path)
            if dist == -1:
                continue
            elif dist <= k:
                list_of_modules.append(filepath)
                module_weight.append(dist)

    # sorting in ascending order
    list_of_modules = [x for _, x in sorted(zip(module_weight, list_of_modules))]
    return list_of_modules


def get_funcs(file, content, target_func, matched_target_function):
    # print(file)
    # Determine the language based on file extension
    ext = get_file_extension(file)
    language = determine_language(ext)
    if language == 'c':
        LANGUAGE = C_LANGUAGE
    elif language == 'cpp':
        LANGUAGE = CPP_LANGUAGE
    else:
        print(f"Language of modified file not recognized for id {id}")
        return

    # Parse the source code with Tree-sitter
    parser = Parser(LANGUAGE)
    tree = parser.parse(bytes(content, 'utf8'))
    node = tree.root_node

    funcs = []

    def traverse(node, matched_target_function, depth=0):
        if depth >= 900:
            return matched_target_function

        if node.type == 'function_definition' or node.type == 'compound_statement' or node.type == 'labeled_statement':
            func_content = node.text.decode('utf-8')
            if func_content != target_func:
                funcs.append(func_content)
            else:
                matched_target_function = True
        else:
            for child in node.children:
                matched_target_function = traverse(child, matched_target_function, depth+1)
        
        return matched_target_function

    matched_target_function = traverse(node, matched_target_function)

    return funcs, matched_target_function


def get_cfc(args, semantic_ranker, project_context):
    # get secure function
    target_func = get_c_cpp_file(f'descriptions/{args.query_id}/sec_func_base')

    status = None
    current_filepath = args.query_original_path
    if len(project_context) == 0:
        cfc_text = ""
        status = "project_not_found"
    else:
        current_filecontent = None
        for filepath, filecontent in project_context.items():
            if filepath == current_filepath:
                current_filecontent = filecontent
                break

        if current_filecontent is None:
            cfc_text = ""
            status = "file_not_found_in_project"

        else:
            pyfiles = find_files_within_distance_k(
                args.query_original_path,
                list(project_context.keys()),
                k=args.crossfile_distance
            )
            pyfiles.insert(0, args.query_original_path)  # consider the current file's functions (but not target func)
            pyfiles = pyfiles[:args.maximum_cross_files]

            c_id = 0
            code_chunks = []
            code_chunk_ids = []
            matched_target_function = False
            for pyfile in pyfiles:
                # get functions (from tree sitter)
                funcs, matched_target_function = get_funcs(pyfile, project_context[pyfile], target_func, matched_target_function)
                for func in funcs:
                    tokenized_func = tokenize_nltk(func)
                    if len(tokenized_func) > 0:
                        code_chunks.append(func)
                        code_chunk_ids.append(f"{pyfile}|{c_id}")
                        c_id += 1
            if not matched_target_function:
                print(f"ID {args.query_id}: target function never found, double check its retrieved funcs")

            if len(code_chunks) == 0:
                cfc_text = ""
                status = "no_crossfile_context"

            else:
                # get masked function
                query = get_c_cpp_file(f'descriptions/{args.query_id}/mask_sec_func_perturbed').strip()

                assert "// <MASK>" in query, "Query should contain the special symbol // <MASK>"
                cfc, cfc_text, meta_data = get_crossfile_context_from_chunks(
                    args=args,
                    query=query,
                    code_chunks=code_chunks,
                    code_chunk_ids=code_chunk_ids,
                    semantic_ranker=semantic_ranker
                )

    return cfc_text, status


def attach_data(args):
    project_context = read_project_files(args.repository_path, args.language)

    semantic_ranker = None
    if args.ranking_fn == "cosine_sim":
        semantic_ranker = SemanticReranking(
            args.ranker,
            max_sequence_length=256
        )

    cfc_text, status = get_cfc(args, semantic_ranker, project_context)
    return cfc_text, status


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--rerank",
        type=str2bool,
        default=True,
        help="rerank the functions"
    )
    parser.add_argument(
        "--ranker",
        type=str,
        default="sparse",
        choices=["sparse", "unixcoder"],
        help="ranking function"
    )
    parser.add_argument(
        "--ranking_fn",
        type=str,
        default="bm25",
        choices=["tfidf", "bm25", "jaccard_sim", "cosine_sim"],
        help="ranking function"
    )
    parser.add_argument(
        "--query_type",
        type=str,
        default="last_n_lines",
        choices=["last_n_lines", "groundtruth"],
        help="how to form query from prompt"
    )
    parser.add_argument(
        "--crossfile_distance",
        type=int,
        default=100,
        help="max distance to search for crossfile"
    )
    parser.add_argument(
        "--maximum_chunk_to_rerank",
        type=int,
        default=5000,
        help="max chunks to consider to rank via BM25"
    )
    parser.add_argument(
        "--maximum_cross_files",
        type=int,
        default=1000,
        help="max chunks to consider to rank via BM25"
    )
    parser.add_argument(
        "--maximum_cross_file_chunk",
        type=int,
        default=5,
        help="max chunks to return as cfc"
    )
    parser.add_argument(
        "--use_next_chunk_as_cfc",
        type=str2bool,
        default=True,
        help="use next code chunk as context"
    )
    parser.add_argument(
        "--skip_if_no_cfc",
        type=str2bool,
        default=True,
        help="skip adding examples if there is no crossfile context"
    )
    parser.add_argument(
        "--output_file_suffix",
        type=str,
        default=None,
        help="add a suffix string to the output file"
    )
    parser.add_argument(
        "--language",
        type=str,
        default="c",
        choices=["java", "python", "typescript", "csharp", "c"],
        help="language name"
    )
    parser.add_argument(
        "--repository_path",
        type=str,
        help="path to the repository"
    )
    parser.add_argument(
        "--query_id",
        type=int,
        help="query id"
    )
    parser.add_argument(
        "--query_original_path",
        type=str,
        help="path to the the original query file"
    )
    args = parser.parse_args()

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    with open('filter_logs/cases.json', 'r') as f:
        cases = json.load(f)
    
    original_path = os.getcwd()
    for id in ids:
        # print(id)
        # if os.path.exists(os.path.join('descriptions', str(id), 'cross-file.txt')):
        #     continue
        args.query_id = id
        args.query_original_path = cases[id]['changed_file']
        commit = cases[id]['fixing_commit']
        repository_root = os.path.join('/space1/cdilgren/project_benchmark/repos', cases[id]['project_name'])
        args.repository_path = repository_root
        # use git to checkout the commit
        try:
            os.chdir(repository_root)
            subprocess.run(['git', 'checkout', commit], check=True)
        except:
            print(f"Failed to checkout {commit} for {id}")
            continue

        os.chdir(original_path)
        cfc_context, status = attach_data(args)

        # print(status)

        with open(os.path.join('descriptions', str(args.query_id), 'cross-file.txt'), "w") as f:
            f.write(cfc_context)