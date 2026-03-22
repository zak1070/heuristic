# Linear Ordering Problem (LOP) - Local Search

## Compilation
Project implemented in C. To compile on Linux/MacOS:
`make clean`
`make`

## Execution
The generated binary is `lop`. It requires an instance file as a parameter.
Example: `./lop --instance instances/N-be75eec_150`

## Automated Data Processing & Analysis
To reproduce the experimental results and statistical tests:
1. Run the bash script to execute all algorithms across all instances. This generates the raw data file:
   `bash run.sh`
2. Run the R script to compute summary statistics (averages, deviations, times) and perform paired Wilcoxon tests:
   `Rscript analyse.R`