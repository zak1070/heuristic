/* Heuristic Optimization assignment.

    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    Completed by M'hamdi Zakaria, 2026.
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file main.c
 * @brief Entry point for the Linear Ordering Problem (LOP) local search solver.
 *
 * This program evaluates various local search algorithms (Iterative Improvement
 * and Variable Neighborhood Descent) on a given LOP instance. It systematically
 * tests combinations of initial solutions (Random, CW), neighborhood structures
 * (Transpose, Exchange, Insert), and pivot rules (First, Best).
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>

#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "optimization.h"

char *FileName;

/**
 * @brief Function pointer type for local search algorithms.
 * * Defines the strict contract that any local search function must follow:
 * @param sol  Pointer to the permutation array (modified in-place).
 * @param cost Pointer to the objective function value (updated in-place).
 */
typedef void (*ls_algorithm_t)(long int* sol, long long int* cost);


/**
 * @brief Parses command-line arguments.
 *
 * Extracts the instance file path provided via the -i or --instance option.
 * Exits the program if the option is missing or unrecognized.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void readOpts(int argc, char **argv) {
    int opt;
    int option_index = 0;
    FileName = NULL;

    static struct option long_options[] = {
        {"instance", required_argument, 0, 'i'},
        {0,          0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "i:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'i':
            FileName = (char *)malloc(strlen(optarg) + 1);
            strcpy(FileName, optarg);
            break;
        case '?':
        default:
            fprintf(stderr, "Unrecognized option.\n");
            exit(1);
        }
    }

    if (!FileName) {
        fprintf(stderr, "No instance file provided (use -i or --instance <instance_name>). Exiting.\n");
        exit(1);
    }
}

/**
 * @brief Evaluates a specific local search algorithm and prints the results.
 *
 * Copies the initial solution, runs the provided local search function,
 * measures the elapsed CPU time, and outputs the performance metrics in csv format.
 *
 * @param inst_name    Name of the instance file.
 * @param init_name    Name of the initialization method used.
 * @param ls_name      Name of the local search method applied.
 * @param initial_sol  Pointer to the initial permutation array.
 * @param initial_cost The objective function value of the initial solution.
 * @param ls_func      Function pointer to the local search procedure.
 */
void evaluate_ls(const char* inst_name, const char* init_name, const char* ls_name, 
                 long int* initial_sol, long long int initial_cost, ls_algorithm_t ls_func) {
                     
    if (!ls_func) {
        fprintf(stderr, "Error: Null local search function pointer provided.\n");
        exit(1);
    }

    long int *working_sol = (long int *)malloc(PSize * sizeof(long int));

    memcpy(working_sol, initial_sol, PSize * sizeof(long int));
    long long int current_cost = initial_cost;
    
    start_timers();
    ls_func(working_sol, &current_cost);
    double elapsed = elapsed_time(VIRTUAL);
    
    printf("%s,%s,%s,%lld,%.6f\n", inst_name, init_name, ls_name, current_cost, elapsed);
    
    free(working_sol);
}

/**
 * @brief Main execution flow.
 */
int main (int argc, char **argv) {
    long int i, j;
    long int *sol_random, *sol_cw;
    long long int cost_random, cost_cw;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s -i <instance_file>\n", argv[0]);
        exit(1);
    }
    
    readOpts(argc, argv);
    CostMat = readInstance(FileName);

    // Deterministic pseudo-random seed based on instance characteristics
    Seed = (long int) 0;
    for (i=0; i < PSize; ++i)
        for (j=0; j < PSize; ++j)
            Seed += (long int) CostMat[i][j];

    char *inst_name = basename(FileName);

    sol_random = (long int *)malloc(PSize * sizeof(long int));
    sol_cw = (long int *)malloc(PSize * sizeof(long int));


    createRandomSolution(sol_random);
    cost_random = computeCost(sol_random);
    
    createCWSolution(sol_cw);
    cost_cw = computeCost(sol_cw);

    // Execute 12 iterative improvement combinations (Random Initialization)
    evaluate_ls(inst_name, "Random", "Transpose_First", sol_random, cost_random, transpose_first_improvement);
    evaluate_ls(inst_name, "Random", "Transpose_Best",  sol_random, cost_random, transpose_best_improvement);
    evaluate_ls(inst_name, "Random", "Exchange_First",  sol_random, cost_random, exchange_first_improvement);
    evaluate_ls(inst_name, "Random", "Exchange_Best",   sol_random, cost_random, exchange_best_improvement);
    evaluate_ls(inst_name, "Random", "Insert_First",    sol_random, cost_random, insert_first_improvement);
    evaluate_ls(inst_name, "Random", "Insert_Best",     sol_random, cost_random, insert_best_improvement);

    // Execute 12 iterative improvement combinations (Chenery-Watanabe Initialization)
    evaluate_ls(inst_name, "CW", "Transpose_First", sol_cw, cost_cw, transpose_first_improvement);
    evaluate_ls(inst_name, "CW", "Transpose_Best",  sol_cw, cost_cw, transpose_best_improvement);
    evaluate_ls(inst_name, "CW", "Exchange_First",  sol_cw, cost_cw, exchange_first_improvement);
    evaluate_ls(inst_name, "CW", "Exchange_Best",   sol_cw, cost_cw, exchange_best_improvement);
    evaluate_ls(inst_name, "CW", "Insert_First",    sol_cw, cost_cw, insert_first_improvement);
    evaluate_ls(inst_name, "CW", "Insert_Best",     sol_cw, cost_cw, insert_best_improvement);

  
    // Execute Variable Neighborhood Descent (VND) variants 
    // (Using CW Initialization and First-improvement rule)

    evaluate_ls(inst_name, "CW", "VND_TEI", sol_cw, cost_cw, vnd_tei);
    evaluate_ls(inst_name, "CW", "VND_TIE", sol_cw, cost_cw, vnd_tie);


    free(sol_random);
    free(sol_cw);
    free(FileName);
    
    return 0;
}