#!/bin/bash
CSV_FILE=$1
LOG_FILE=$2

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
python arvo.py eval "${ID_LIST[@]}" --path /data/fyj-oss-fuzz-bench/ --output /data/fyj-oss-fuzz-bench/output/ >> "$LOG_FILE" 2>&1