# Running SecRepoBench

## Run Inference
The model names can be found in constants.py
The prompt_types can be no-security-reminder, sec-generic, sec-specific, or security-policy
The context_types can be BM25, dense-file, or in-file

```
python run_inference.py --model_names [YOUR_MODEL_NAMES] --prompt_types [YOUR_PROMPT_TYPES] --context_types [YOUR_CONTEXT_TYPES] [--rerun]
```

Code completions are saved in the completions directory.

## Run Evaluation
```
python run_eval.py --model_names [YOUR_MODEL_NAMES] --prompt_types [YOUR_PROMPT_TYPES] --context_types [YOUR_CONTEXT_TYPES] [--rerun]
```

Results are saved in the eval_report directory.
