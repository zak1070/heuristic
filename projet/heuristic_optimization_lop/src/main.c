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

void readOpts(int argc, char **argv) {
    int opt;
    int option_index = 0;
    FileName = NULL;

    // Définition des options longues
    static struct option long_options[] = {
        {"instance", required_argument, 0, 'i'},
        {0,          0,                 0,  0 }
    };


    while ((opt = getopt_long(argc, argv, "i:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'i':
            FileName = (char *)malloc(strlen(optarg) + 1);
            //TODO voir si on garde derniere version en bas 
            strcpy(FileName, optarg);
            // strncpy(FileName, optarg, strlen(optarg));
            break;
        case '?':
        default:
            fprintf(stderr, "Option non reconnue.\n");
            exit(1);
        }
    }

    if (!FileName) {
        fprintf(stderr, "No instance file provided (use -i or --instance <instance_name>). Exiting.\n");
        exit(1);
    }
}

void evaluate_ls(const char* inst_name, const char* init_name, const char* ls_name, long int* initial_sol, long long int initial_cost, void (*ls_func)(long int*, long long int*)) {
    long int *working_sol = (long int *)malloc(PSize * sizeof(long int));
    memcpy(working_sol, initial_sol, PSize * sizeof(long int));
    
    long long int current_cost = initial_cost;
    start_timers();
    ls_func(working_sol, &current_cost);
    double elapsed = elapsed_time(VIRTUAL);
    
    printf("%s,%s,%s,%lld,%.6f\n", inst_name, init_name, ls_name, current_cost, elapsed);
    free(working_sol);
}

int main (int argc, char **argv) {
    long int i, j;
    long int *sol_random, *sol_cw;
    long long int cost_random, cost_cw;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    if (argc < 2) exit(1);
    
    readOpts(argc, argv);
    CostMat = readInstance(FileName);

    Seed = (long int) 0;
    for (i=0; i < PSize; ++i)
        for (j=0; j < PSize; ++j)
            Seed += (long int) CostMat[i][j];

    char *inst_name = basename(FileName);

    sol_random = (long int *)malloc(PSize * sizeof(long int));
    sol_cw = (long int *)malloc(PSize * sizeof(long int));

    // Initialisation
    createRandomSolution(sol_random);
    cost_random = computeCost(sol_random);
    
    createCWSolution(sol_cw);
    cost_cw = computeCost(sol_cw);

    // Exécution des 12 combinaisons (Random)
    evaluate_ls(inst_name, "Random", "Transpose_First", sol_random, cost_random, transpose_first_improvement);
    evaluate_ls(inst_name, "Random", "Transpose_Best",  sol_random, cost_random, transpose_best_improvement);
    evaluate_ls(inst_name, "Random", "Exchange_First",  sol_random, cost_random, exchange_first_improvement);
    evaluate_ls(inst_name, "Random", "Exchange_Best",   sol_random, cost_random, exchange_best_improvement);
    evaluate_ls(inst_name, "Random", "Insert_First",    sol_random, cost_random, insert_first_improvement);
    evaluate_ls(inst_name, "Random", "Insert_Best",     sol_random, cost_random, insert_best_improvement);

    // Exécution des 12 combinaisons (CW)
    evaluate_ls(inst_name, "CW", "Transpose_First", sol_cw, cost_cw, transpose_first_improvement);
    evaluate_ls(inst_name, "CW", "Transpose_Best",  sol_cw, cost_cw, transpose_best_improvement);
    evaluate_ls(inst_name, "CW", "Exchange_First",  sol_cw, cost_cw, exchange_first_improvement);
    evaluate_ls(inst_name, "CW", "Exchange_Best",   sol_cw, cost_cw, exchange_best_improvement);
    evaluate_ls(inst_name, "CW", "Insert_First",    sol_cw, cost_cw, insert_first_improvement);
    evaluate_ls(inst_name, "CW", "Insert_Best",     sol_cw, cost_cw, insert_best_improvement);

    // =================================================================
    // Exécution des 2 VND (Chenery-Watanabe, First-improvement)
    // =================================================================
    evaluate_ls(inst_name, "CW", "VND_TEI", sol_cw, cost_cw, vnd_tei);
    evaluate_ls(inst_name, "CW", "VND_TIE", sol_cw, cost_cw, vnd_tie);

    free(sol_random);
    free(sol_cw);
    free(FileName);
    return 0;
}

// /*  Heuristic Optimization assignment, 2015.
//     Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
//     of Tommaso Schiavinotto:
//     ---
//     ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
//     Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <http://www.gnu.org/licenses/>.
// */

// #include <stdio.h>
// #include <stdlib.h>
// #include <getopt.h>
// #include <string.h>

// #include "instance.h"
// #include "utilities.h"
// #include "timer.h"
// #include "optimization.h"

// char *FileName;

// void readOpts(int argc, char **argv) {
//   char opt;

//   FileName = NULL;
//   while ( (opt = getopt(argc, argv, "i:")) > 0 )  
//       switch (opt) {
// 	  case 'i': /* Instance file */
// 	      FileName = (char *)malloc(strlen(optarg)+1);
// 	      strncpy(FileName, optarg, strlen(optarg));
// 	      break;
// 	  default:
// 	      fprintf(stderr, "Option %c not managed.\n", opt);
//       }
    
//   if ( !FileName ) {
//     printf("No instance file provided (use -i <instance_name>). Exiting.\n");
//     exit(1);
//   }
// }



// int main (int argc, char **argv) 
// {
//   long int i,j;
//   long int *currentSolution;
//   long long int cost, current_cost;

//   /* Do not buffer output */
//   setbuf(stdout,NULL);
//   setbuf(stderr,NULL);
  
//   if (argc < 2) {
//     printf("No instance file provided (use -i <instance_name>). Exiting.\n");
//     exit(1);
//   }
  
//   /* Read parameters */
//   readOpts(argc, argv);

//   /* Read instance file */
//   CostMat = readInstance(FileName);
//   printf("Data have been read from instance file. Size of instance = %ld.\n\n", PSize);

//   /* initialize random number generator, deterministically based on instance.
//    * To do this we simply set the seed to the sum of elements in the matrix, so it is constant per-instance,
//    but (most likely) varies between instances */
//   Seed = (long int) 0;
//     for (i=0; i < PSize; ++i)
//       for (j=0; j < PSize; ++j)
//         Seed += (long int) CostMat[i][j];
//   printf("Seed used to initialize RNG: %ld.\n\n", Seed);
  
//   /* starts time measurement */
//   start_timers();

//   /* A solution is just a vector of int with the same size as the instance */
//   currentSolution = (long int *)malloc(PSize * sizeof(long int));

//   /* Create an initial random solution. 
//      The only constraint is that it should always be a permutation */
//   createCWSolution(currentSolution);

//   /* Print solution */
//   printf("Initial solution:\n");
//   for (j=0; j < PSize; j++) 
//     printf(" %ld", currentSolution[j]);
//   printf("\n");

//   /* Compute cost of solution and print it */
//   cost = computeCost(currentSolution);
//   printf("Cost of this initial solution: %lld\n\n", cost);

//  /* Lancement de la recherche locale */
//   printf("Lancement de  insert Best-Improvement...\n");
//   current_cost = cost;
  
//   // Appel de la fonction (assure-toi qu'elle est bien déclarée dans optimization.h)
//   insert_best_improvement(currentSolution, &current_cost);
  
//   printf("Cout de la solution apres  insert Best-Improvement: %lld\n", current_cost);
//   printf("Time elapsed since we started the timer: %g\n\n", elapsed_time(VIRTUAL));



//   return 0;
// }

// // Pour tester toutes les algo sur la meme base
// // int main (int argc, char **argv) {
// //   long int i, j;
// //   long int *initialSolution, *workingSolution;
// //   long long int initial_cost, current_cost;

// //   setbuf(stdout, NULL);
// //   setbuf(stderr, NULL);
  
// //   if (argc < 2) {
// //     printf("No instance file provided (use -i <instance_name>). Exiting.\n");
// //     exit(1);
// //   }
  
// //   readOpts(argc, argv);
// //   CostMat = readInstance(FileName);
// //   printf("Taille de l'instance = %ld.\n", PSize);

// //   Seed = (long int) 0;
// //   for (i=0; i < PSize; ++i)
// //     for (j=0; j < PSize; ++j)
// //       Seed += (long int) CostMat[i][j];

// //   // Allocation de deux tableaux : un pour conserver l'état initial, un pour travailler
// //   initialSolution = (long int *)malloc(PSize * sizeof(long int));
// //   workingSolution = (long int *)malloc(PSize * sizeof(long int));

// //   // Génération de la solution de départ
// //   createCWSolution(initialSolution);
// //   initial_cost = computeCost(initialSolution);
// //   printf("Cout initial (Chenery-Watanabe) : %lld\n\n", initial_cost);

// //   // --- TEST 1 : TRANSPOSE BEST ---
// //   memcpy(workingSolution, initialSolution, PSize * sizeof(long int));
// //   current_cost = initial_cost;
// //   start_timers();
// //   transpose_best_improvement(workingSolution, &current_cost);
// //   printf("[Transpose Best] Cout : %lld | Temps : %g s\n", current_cost, elapsed_time(VIRTUAL));

// //   // --- TEST 2 : EXCHANGE BEST ---
// //   memcpy(workingSolution, initialSolution, PSize * sizeof(long int));
// //   current_cost = initial_cost;
// //   start_timers();
// //   exchange_best_improvement(workingSolution, &current_cost);
// //   printf("[Exchange Best]  Cout : %lld | Temps : %g s\n", current_cost, elapsed_time(VIRTUAL));

// //   // --- TEST 3 : INSERT BEST ---
// //   memcpy(workingSolution, initialSolution, PSize * sizeof(long int));
// //   current_cost = initial_cost;
// //   start_timers();
// //   insert_best_improvement(workingSolution, &current_cost);
// //   printf("[Insert Best]    Cout : %lld | Temps : %g s\n", current_cost, elapsed_time(VIRTUAL));

// //   free(initialSolution);
// //   free(workingSolution);
// //   return 0;
// // }
