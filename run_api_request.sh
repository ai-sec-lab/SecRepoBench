#!/bin/bash

# Lists of parameters to test
MODELS=("gpt-4o" "claude-3.5-sonnet" "gemini-1.5-flash" "gpt-4o-mini" "claude-3-haiku" "gemini-1.5-pro")
CONTEXTS=("cross-file in-file")
PROMPTS=("sec-generic" "sec-specific" "system-prompt")
MODES=("perturbed")

# Loop through all combinations
for PROMPT in "${PROMPTS[@]}"; do
    for CONTEXT in "${CONTEXTS[@]}"; do
        for MODEL in "${MODELS[@]}"; do
            for MODE in "${MODES[@]}"; do
                echo "Running experiment with:"
                echo "  Model: $MODEL"
                echo "  Context: $CONTEXT"
                echo "  Prompt: $PROMPT"
                echo "  Mode: $MODE"
                
                python api_request.py \
                    --model_name "$MODEL" \
                    --context_type "$CONTEXT" \
                    --prompt_type "$PROMPT" \
                    --mode "$MODE"
                
                # Add a small delay between requests to avoid rate limiting
                sleep 2
                
                echo "----------------------------------------"
            done
        done
    done
done