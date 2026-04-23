#!/bin/bash

# Create the header (overwrites any existing file)
echo "Instance,Init,Algorithm,Cost,Time_s" > raw_data.txt

# Run the C program on all instances
# Make sure the path to your instances matches your directory structure
for file in instances/N-*; do
    ./lop --instance "$file" >> raw_data.txt
done

# File paths
BEST_KNOWN="./best_known/best_known.txt"
INPUT_FILE="raw_data.txt"
OUTPUT_FILE="final_data.txt"

if [ ! -f "$BEST_KNOWN" ]; then
    echo "Error: The file $BEST_KNOWN was not found."
    exit 1
fi


echo "Instance,Init,Algorithm,Cost,Time_s,Deviation_%" > "$OUTPUT_FILE"

# Read the input txt file line by line
while IFS=',' read -r instance init algo cost time_s; do

  
    if [[ "$instance" == "Instance" ]] || [[ -z "$instance" ]]; then
        continue
    fi

  
    instance_name=$(basename "$instance")

    # Extract the optimal cost from best_known.txt
    best_cost=$(grep -w "^${instance_name}" "$BEST_KNOWN" | awk '{print $2}')

    if [ -n "$best_cost" ] && [ "$best_cost" -ne 0 ]; then
        # Calculate the deviation for a MAXIMIZATION problem: ((Best - Cost) / Best) * 100
        deviation=$(awk -v b="$best_cost" -v c="$cost" 'BEGIN { printf "%.6f", ((b - c) / b) * 100 }')
        
       
        echo "${instance_name},${init},${algo},${cost},${time_s},${deviation}" >> "$OUTPUT_FILE"
    fi
done < "$INPUT_FILE"

echo "Processing complete. The final data is stored in $OUTPUT_FILE"