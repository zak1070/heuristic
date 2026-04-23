# ==============================================================================
# Makefile for Heuristic Optimization - LOP Solver (Part 1 & 2)
# ==============================================================================

CC      = gcc
CFLAGS  = -O3 -Wall -Wextra -std=c99
LDFLAGS = -lm

# Common source files
COMMON_SRC = src/instance.c src/optimization.c src/timer.c src/utilities.c
COMMON_OBJ = $(COMMON_SRC:.c=.o)

# Part 1: Iterative Improvement
PART1_SRC  = src/main.c
PART1_OBJ  = $(PART1_SRC:.c=.o)
PART1_BIN  = lop_ls

# Part 2: Stochastic Local Search (ILS & MA)
PART2_SRC  = src/sls.c src/main2.c
PART2_OBJ  = $(PART2_SRC:.c=.o)
PART2_BIN  = lop_sls

.PHONY: all clean clean-all run run-sls analyze doc part1 part2

# Default target
all: part1 part2

part1: $(PART1_BIN)
part2: $(PART2_BIN)

$(PART1_BIN): $(COMMON_OBJ) $(PART1_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PART2_BIN): $(COMMON_OBJ) $(PART2_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean object files and executables
clean:
	rm -f src/*.o $(PART1_BIN) $(PART2_BIN)

# Clean everything including generated data files
clean-all: clean
	rm -f simple.txt raw_data.txt final_data.txt results_main.csv results_rtd.csv

# Run Part 1 experiments
run: part1
	chmod +x generate_results.sh
	./generate_results.sh

# Run Part 2 experiments
run-sls: part2
	chmod +x generate_sls_results.sh
	./generate_sls_results.sh

# Run statistical analysis (R scripts)
analyze:
	Rscript analyse.R
	Rscript analyse2.R

# Generate PDF report
doc:
	pdflatex report.tex