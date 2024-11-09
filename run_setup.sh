#!/bin/bash

# Define paths
CSV_FILE=$1

# Iterate over each ID in the CSV file
tail -n +2 "$CSV_FILE" | while IFS=',' read -r ID; do
    # Execute the command for each ID and append the output to the log file
    python arvo.py setup "$ID" --path /data/luke/oss-fuzz-bench
done