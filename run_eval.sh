#!/bin/bash
CSV_FILE=$1
LOG_FILE=$2
# LOG_FILE_2=$3
# LOG_FILE_3=$4

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

# Print the list of IDs to the log file
echo "Processing IDs: ${ID_LIST[@]}" >> "$LOG_FILE"

# Run the eval command with all IDs in one go
# claude-3-haiku claude-3.5-sonnet gemini-1.5-flash gemini-1.5-pro gpt-4o-mini gpt-4o
python eval.py eval "${ID_LIST[@]}" \
    --path /data/oss-fuzz-bench \
    --output /data/oss-fuzz-bench/output \
    --tests unittest testcase \
    --rerun \
    --model_names llama-3.1-70b-instruct \
    --context_types cross-file \
    --prompt_types sec-generic \
    --modes perturbed >> "$LOG_FILE" 2>&1

# echo "Processing IDs: ${ID_LIST[@]}" >> "$LOG_FILE_2"
# python eval.py eval "${ID_LIST[@]}" \
#     --path /data/oss-fuzz-bench \
#     --output /data/oss-fuzz-bench/output \
#     --tests unittest testcase \
#     --rerun \
#     --model_names llama-3.1-70b-instruct \
#     --context_types cross-file \
#     --prompt_types sec-specific \
#     --modes perturbed >> "$LOG_FILE_2" 2>&1

# echo "Processing IDs: ${ID_LIST[@]}" >> "$LOG_FILE_3"
# python eval.py eval "${ID_LIST[@]}" \
#     --path /data/oss-fuzz-bench \
#     --output /data/oss-fuzz-bench/output \
#     --tests unittest testcase \
#     --rerun \
#     --model_names llama-3.1-70b-instruct \
#     --context_types cross-file \
#     --prompt_types system-prompt \
#     --modes perturbed >> "$LOG_FILE_3" 2>&1

# echo "Processing IDs: ${ID_LIST[@]}" >> "$LOG_FILE_4"
# python eval.py eval "${ID_LIST[@]}" \
#     --path /data/oss-fuzz-bench \
#     --output /data/oss-fuzz-bench/output \
#     --tests unittest testcase \
#     --rerun \
#     --model_names claude-3-haiku claude-3.5-sonnet gemini-1.5-flash gemini-1.5-pro gpt-4o-mini gpt-4o \
#     --context_types in-file \
#     --prompt_types sec-generic \
#     --modes perturbed >> "$LOG_FILE_4" 2>&1

# echo "Processing IDs: ${ID_LIST[@]}" >> "$LOG_FILE_5"
# python eval.py eval "${ID_LIST[@]}" \
#     --path /data/oss-fuzz-bench \
#     --output /data/oss-fuzz-bench/output \
#     --tests unittest testcase \
#     --rerun \
#     --model_names claude-3-haiku claude-3.5-sonnet gemini-1.5-flash gemini-1.5-pro gpt-4o-mini gpt-4o \
#     --context_types in-file \
#     --prompt_types sec-specific \
#     --modes perturbed >> "$LOG_FILE_5" 2>&1
