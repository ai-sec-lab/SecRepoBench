# 🛡️ SecRepoBench

## 🚀 Running Inference

To run inference using SecRepoBench:

```bash
python run_inference.py \
  --model_names [YOUR_MODEL_NAMES] \
  --prompt_types [YOUR_PROMPT_TYPES] \
  --context_types [YOUR_CONTEXT_TYPES] \
  [--rerun]
```

- **Model names**: Defined in `constants.py`
- **Prompt types**:
  - `no-security-reminder`
  - `sec-generic`
  - `sec-specific`
  - `security-policy`
- **Context types**:
  - `BM25`
  - `dense-file`
  - `in-file`

📁 *Code completions are saved in the `completions/` directory.*

---

## 📊 Running Evaluation

To evaluate the model completions:

```bash
python run_eval.py \
  --model_names [YOUR_MODEL_NAMES] \
  --prompt_types [YOUR_PROMPT_TYPES] \
  --context_types [YOUR_CONTEXT_TYPES] \
  [--rerun]
```

📁 *Evaluation results are saved in the `eval_results/` directory.*
