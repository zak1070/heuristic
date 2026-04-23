# Heuristic Optimization: Linear Ordering Problem (LOP)

This repository contains the C implementation of Iterative Improvement and Variable Neighborhood Descent (VND) algorithms to solve the Linear Ordering Problem (LOP). This project is part of the first implementation exercise for the Heuristic Optimization course.

## System Requirements

To compile and execute this project, the following tools must be installed on your Linux/MacOS environment:
- **GCC** (GNU Compiler Collection)
- **GNU Make**
- **Bash** (for execution automation)
- **R** (for statistical analysis)

## Project Structure

- `src/` : Contains all C source code files (`.c`) and headers (`.h`).
- `instances/` : Directory containing the LOP instances to be evaluated.
- `best_known/` : Directory containing the `best_known.txt` file with optimal costs.
- `Makefile` : Build automation script.
- `run.sh` : Bash script automating the execution and data formatting.
- `analyse.R` : R script performing statistical tests (Student's t-test and Wilcoxon).

## Compilation

To compile the C source code with maximum performance optimization (`-O3`), open your terminal in the project's root directory and run:

```bash
make
```
This command links the object files and generates the `lop` executable.

## Execution Pipeline

The execution workflow is fully automated and split into two distinct steps: Data Generation and Statistical Analysis.

### 1. Generating Raw Data
To execute the `lop` solver across all instances located in the `instances/` folder and compute the objective deviations, run:

```bash
make run
```
*(Alternatively, you can manually run `./run.sh`)*

This script automatically handles the iterations and outputs the final metrics into `final_stats.txt`.

### 2. Statistical Analysis
To process the generated `final_stats.txt` file and print the summary tables alongside the required hypothesis tests (Exercise 1.1 and 1.2), run:

```bash
make analyze
```
*(Alternatively, you can manually run `Rscript analyse.R`)*

## Clean-up Commands

To remove the compiled `.o` object files and the `lop` executable:
```bash
make clean
```

To perform a complete clean-up (removes compiled files as well as all generated TXT data files):
```bash
make clean-all
```
