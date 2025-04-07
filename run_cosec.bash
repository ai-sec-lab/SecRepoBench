#!/bin/bash

# Simple script to run inference with CosecEvaler only

# Set parameters for CosecEvaler
MODELS=("deepseek-coder-6.7b-instruct")
CONTEXT="in-file-truncated"
PROMPT="system-prompt"
MODE="perturbed"
FINAL_MODEL="/space1/pchiniya/CoSec/sven/trained/deepseek/final_merged"
for MODEL in "${MODELS[@]}"; do
    
    python api_request_cosec.py \
    --evaler cosec \
    --model_name "$MODEL" \
    --context_type "$CONTEXT" \
    --prompt_type "$PROMPT" \
    --mode "$MODE" \
    --final_model "$FINAL_MODEL"

    # Add a small delay between requests to avoid rate limiting
    sleep 2
    
    echo "----------------------------------------"
done