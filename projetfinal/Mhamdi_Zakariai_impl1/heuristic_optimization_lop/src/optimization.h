/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    Completed by M'hamdi Zakaria, Computer Science student at ULB.
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
 * @file optimization.h
 * @brief Declarations for initialization, local search, and VND algorithms for the LOP.
 * * This module implements iterative improvement algorithms with speed-ups 
 * (incremental updates) for the Linear Ordering Problem[cite: 11, 15].
 */
#ifndef _LO_H_
#define _LO_H_


extern long int **CostMat;


long long int computeCost(long int *solution);
void createRandomSolution(long int *solution);

/* Exercise 1.1: Initializations and Local Search */
void createCWSolution(long int *solution);
void transpose_first_improvement(long int *solution, long long int *current_cost);
void transpose_best_improvement(long int *solution, long long int *current_cost);
void exchange_first_improvement(long int *solution, long long int *current_cost);
void exchange_best_improvement(long int *solution, long long int *current_cost);
void insert_first_improvement(long int *solution, long long int *current_cost);
void insert_best_improvement(long int *solution, long long int *current_cost);


/* Exercise 1.2: Variable Neighborhood Descent */
void vnd_tei(long int *solution, long long int *current_cost);
void vnd_tie(long int *solution, long long int *current_cost);

#endif
