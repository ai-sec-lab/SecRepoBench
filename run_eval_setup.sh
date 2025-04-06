#!/bin/bash

# Define paths
CSV_FILE=$1

# Initialize an empty array to store IDs
ID_LIST=()

# Read the CSV file, skipping the header (first line)
{
    read # Skip the header line
    while IFS=',' read -r ID; do
        # Append the ID to the list
        ID_LIST+=("$ID")
    done
} < "$CSV_FILE"

# Check if ID_LIST is populated
if [ ${#ID_LIST[@]} -eq 0 ]; then
    echo "No IDs found. Exiting." >> "$LOG_FILE"
    exit 1
fi

# Run the eval command with all IDs in one go
python eval.py setup "${ID_LIST[@]}" --path /home/cdilgren/project_benchmark/oss-fuzz-bench \
    --model_names llama-3.1-70b-instruct \
    --context_types cross-file \
    --prompt_types system-prompt sec-generic sec-specific \
    --modes perturbed
