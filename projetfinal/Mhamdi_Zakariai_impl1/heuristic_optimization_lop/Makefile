# ==============================================================================
# Makefile for Heuristic Optimization - LOP Solver
# ==============================================================================


CC = gcc
CFLAGS = -O3 -Wall


OBJECTS = src/instance.o src/main.o src/optimization.o src/timer.o src/utilities.o


.PHONY: all clean run analyze clean-all


all: lop


lop: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o lop


clean:
	rm -f src/*~ src/*.o lop


run: lop
	chmod +x generate_results.sh
	./generate_results.sh

analyze:
	Rscript analyse.R


clean-all: clean
	rm -f raw_data.txt final_data.txt

doc:
	pdflatex doc/report.tex