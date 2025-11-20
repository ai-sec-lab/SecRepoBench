import argparse
from dotenv import load_dotenv
import sys
from tools.preprocessor import run_inferences, docker_setup

load_dotenv()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--agents", nargs='+',
                        help="List of agents to evaluate")
    parser.add_argument('--model-names', nargs='+', type=str)
    parser.add_argument('--context-types', nargs='+', type=str)
    parser.add_argument('--prompt-types', nargs='+', type=str)
    parser.add_argument('--rerun', action="store_true",
                        help="With the rerun flag, it will run inference for a task even if it is in the cache. Otherwise, it will not.")
    args = parser.parse_args()

    with open('assets/ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]

    max_workers = 5  # Adjust based on API limits

    modes = ["perturbed"]

    for agent in args.agents:
        if agent != "none" and agent != "claudecode":
            docker_setup(ids, max_workers, args.rerun)

    run_inferences(ids, args.agents, args.model_names, args.prompt_types,
                   args.context_types, args.rerun, modes, max_workers)


if __name__ == "__main__":
    sys.argv = [
        "run_inference.py",
        "--agents", "openhands",
        "--model-names", "o3",
        "--context-types", "BM25",  # bm25
        # no-security-reminder security-policy
        "--prompt-types", "no-security-reminder",
        # "--rerun"
    ]
    main()
