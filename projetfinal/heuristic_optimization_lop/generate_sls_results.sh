#!/bin/bash

# Executable for Part 2
EXEC="./lop_sls" 

echo "================================================="
echo " Phase 1: Dynamic VND Time Calibration (-v)      "
echo "================================================="

total_time=0
count=0

for inst in instances/*_150; do
    if [ ! -f "$inst" ]; then continue; fi

    output=$($EXEC -i "$inst" -v) 
    time=$(echo "$output" | grep "VND_TIME:" | awk '{print $2}')

    if [ -n "$time" ]; then
        total_time=$(awk "BEGIN {print $total_time + $time}")
        count=$((count + 1))
        echo " -> Processed $(basename "$inst") : $time s"
    fi
done

if [ $count -eq 0 ]; then
    echo "[Error] No VND time measured. Check executable compilation."
    exit 1
fi

AVG_TIME=$(awk "BEGIN {print $total_time / $count}")
TIME_LIMIT=$(awk "BEGIN { printf \"%.0f\", $AVG_TIME * 500 }")
RTD_LIMIT=$(awk "BEGIN { printf \"%.0f\", $AVG_TIME * 5000 }") 

echo "-------------------------------------------------"
echo " Average VND Time      : $AVG_TIME s"
echo " Main Evaluation Limit : $TIME_LIMIT s (x500)"
echo " RTD Evaluation Limit  : $RTD_LIMIT s (x5000)"
echo "-------------------------------------------------"

echo "================================================="
echo " Phase 2: Main Evaluation (-t $AVG_TIME)         "
echo "================================================="
echo "Instance,Algorithm,Cost,Time_s" > results_main.csv

for inst in instances/*_150; do
    # CORRECTION : Envoi de $AVG_TIME au lieu de $TIME_LIMIT
    $EXEC -i "$inst" -t "$AVG_TIME" -r 1 >> results_main.csv
done
echo " -> results_main.csv generated successfully."

echo "================================================="
echo " Phase 3: RTD Evaluation (-t $AVG_TIME)          "
echo "================================================="
echo "Instance,Algorithm,Repetition,Cost,Time_s" > results_rtd.csv



# Cibles = 0.5% worse than Best Known
TARGET_EEC=3465413
TARGET_NP=7138453

$EXEC -i instances/N-be75eec_150 -t "$AVG_TIME" -b $TARGET_EEC -r 25 >> results_rtd.csv
$EXEC -i instances/N-be75np_150 -t "$AVG_TIME" -b $TARGET_NP -r 25 >> results_rtd.csv
echo " -> results_rtd.csv generated successfully."

echo "================================================="
echo " Execution completed successfully.               "
echo " You may now execute: Rscript analyse2.R         "
echo "================================================="
