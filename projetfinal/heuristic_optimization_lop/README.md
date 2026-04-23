# Heuristic Optimization: Linear Ordering Problem (LOP)

This repository contains the C implementation of Iterative Improvement algorithms (Part 1) and Stochastic Local Search (SLS) metaheuristics (Part 2) to solve the Linear Ordering Problem (LOP). This project is part of the implementation exercises for the Heuristic Optimization course (INFO-H-413).

## System Requirements

To compile and execute this project, the following tools must be installed on your Linux/MacOS environment (or Windows via WSL):
- **GCC** (GNU Compiler Collection)
- **GNU Make**
- **Bash** (for execution automation)
- **R** (for statistical analysis and plotting)

## Project Structure

- `src/` : Contains all C source code files and headers.
  - `optimization.[c|h]` : Core local search and VND functions (Part 1).
  - `sls.[c|h]` : ILS and Memetic Algorithm implementations (Part 2).
  - `main.c` / `main2.c` : Entry points for Part 1 and Part 2.
- `instances/` : Directory containing the LOP benchmark instances (`.mat`).
- `best_known/` : Directory containing the optimal reference costs (`best_known.csv` or `.txt`).
- `Makefile` : Build automation script.
- `run.sh` : Bash script automating the execution and data generation.
- `analyse.R` / `analyse2.R` : R scripts performing statistical tests (Student's t-test, Wilcoxon) and generating Run-Time Distribution (RTD) plots.

## Compilation

The project uses the `-O3` compiler flag to ensure maximum performance, coupled with `static inline` delta-evaluations. To compile the source code, open your terminal in the project's root directory and run:

```bash
# Build both executables (lop_ls and lop_sls)
make all

# Clean compiled objects
make clean
```

## Execution Pipeline

### 1. Part 1: Iterative Improvement & VND
The executable generated for this part is `lop_ls`.

To execute the solver across all instances and compute the objective deviations, run the automated script:
```bash
make run
```
*(Alternatively, you can manually run `./run.sh`).*
This script automatically handles the iterations and outputs the final metrics into `simple.txt` (as requested in the assignment guidelines).

### 2. Part 2: Stochastic Local Search (ILS & MA)
The executable generated for this part is `lop_sls`. The output is formatted strictly as CSV.

**Main Evaluation (1 run per instance, time limit = 500 * T_vnd):**
```bash
./lop_sls -i instances/<instance_name>.mat -t 60 -m >> results_main.csv
```

**Run-Time Distribution Evaluation (25 repetitions, time limit = 5000 * T_vnd):**
```bash
./lop_sls -i instances/<instance_name>.mat -t 300 -r 25 >> results_rtd.csv
```

## Statistical Analysis

To process the generated data files, print the summary tables, perform the hypothesis tests, and generate correlation/RTD plots, run the R scripts:

```bash
# Part 1 Analysis (reads simple.txt)
Rscript analyse.R

# Part 2 Analysis (reads results_main.csv and results_rtd.csv)
Rscript analyse2.R
```

## SLS Algorithm Parameters (Part 2)

| Algorithm | Parameter | Value | Justification |
| :--- | :--- | :--- | :--- |
| **ILS** | Perturbation Strength | 8 | Follows Schiavinotto & Stützle (2004) standard of random interchanges to escape deep local optima. |
| **ILS** | Acceptance Epsilon | 0.0001 | Implements the "Accept small worsening" criterion to favor exploration. |
| **MA** | Population Size | 20 | Standard population size balancing diversity and computational speed for the LOP. |
| **MA** | Crossover Operator | OB | Order-Based (OB) crossover preserves relative order, which is optimal for ranking problems. |
| **MA** | Diversification Limit | 20 gens | Triggers a partial restart if the best solution does not improve, preventing premature convergence. |

## Clean-up Commands

To perform a complete clean-up (removes compiled `.o` files, executables, and all generated `.txt`/`.csv` data files):
```bash
make clean-all
```