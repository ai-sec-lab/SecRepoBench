import argparse

from analyze_eval_report import analyze_report
from eval import eval_setup, eval


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model_names", nargs='+', help="List of models to evaluate")
    parser.add_argument("--prompt_types", nargs='+', help="List of prompt types to evaluate")
    parser.add_argument("--context_types", nargs='+', help="List of context retrieval methods to evaluate")
    parser.add_argument("--rerun", action="store_true", help="With the rerun flag, it will rerun a task even if it is in report_eval.json. Otherwise, it will not.")

    args = parser.parse_args()

    # modes refers to the code mutation strategy to mitigate memorization. 
    # We only have the local var perturbation ('perturbed' mode).
    modes = ['perturbed']

    # consider exposing num_workers
    num_workers = 16

    # evaluate all ids in ids.txt
    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]
    ids = ['910']

    # setup files for eval
    # eval_setup(ids, args.model_names, args.prompt_types, args.context_types, modes, num_workers)

    # run eval for all ids in ids.txt
    eval_report = eval(ids, args.model_names, args.prompt_types, args.context_types, modes, args.rerun, num_workers)

    # process results
    analyze_report(ids, eval_report)

if __name__ == "__main__":
    main()
