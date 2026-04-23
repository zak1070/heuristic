/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
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
#include <stdio.h>
#include <stdlib.h>

#include "optimization.h"
#include "instance.h"
#include "utilities.h"

// Global cost matrix: CostMat[i][j] = cost of placing element i before element j 
long int **CostMat;


long long int computeCost(long int *solution) {
    long int row, col;
    long long int total_cost;

    for (total_cost = 0, row = 0; row < PSize; row++)
        for (col = row + 1; col < PSize; col++)
            total_cost += CostMat[solution[row]][solution[col]];
    return total_cost;
}



void createRandomSolution(long int *solution) {
    int position;
    long int *random_permutation;

    random_permutation = generate_random_vector(PSize);
    if (!random_permutation) fatal("Memory allocation error");
    for (position = 0; position < PSize; position++)
        solution[position] = random_permutation[position];
    free(random_permutation);
}


typedef struct {
    long int      element_id;
    long long int attractiveness;
} CWItem;


static int compare_cw(const void *first, const void *second) {
    long long int score_first  = ((CWItem *)first)->attractiveness;
    long long int score_second = ((CWItem *)second)->attractiveness;
    if (score_first < score_second) return  1;
    if (score_first > score_second) return -1;
    return 0;
}

/*
 * Chenery-Watanabe heuristic: ranks elements by their attractiveness score
 * score(i) = sum_{j!=i} [ CostMat[i][j] - CostMat[j][i] ]
 * Elements with a higher score are placed earlier in the permutation.
 */
void createCWSolution(long int *solution) {
    CWItem *items = malloc(PSize * sizeof(CWItem));
    if (!items) fatal("Memory allocation error for CWItem array");

    for (long int element = 0; element < PSize; element++) {
        items[element].element_id     = element;
        items[element].attractiveness = 0;
        for (long int other = 0; other < PSize; other++) {
            if (element != other) {
                items[element].attractiveness += CostMat[element][other];
                items[element].attractiveness -= CostMat[other][element];
            }
        }
    }

    qsort(items, PSize, sizeof(CWItem), compare_cw);

    for (long int position = 0; position < PSize; position++)
        solution[position] = items[position].element_id;

    free(items);
}


//---- Transpose Neighborhood ---- 

/*
 * Swapping adjacent elements (pos and pos+1) only affects their relative order.
 * Delta evaluation complexity: O(1).
 */
void transpose_first_improvement(long int *solution, long long int *current_cost) {
    int improved = 1;
    while (improved) {
        improved = 0;
        for (long int pos = 0; pos < PSize - 1; pos++) {
            long int left_element  = solution[pos];
            long int right_element = solution[pos + 1];

            //Cost variation: add new reversed arc, subtract old forward arc 
            long long int delta    = CostMat[right_element][left_element]
                                   - CostMat[left_element][right_element];

            if (delta > 0) {
                solution[pos]     = right_element;
                solution[pos + 1] = left_element;
                *current_cost    += delta;
                improved          = 1;
                break; // First-improvement: restart search immediately 
            }
        }
    }
}

void transpose_best_improvement(long int *solution, long long int *current_cost) {
    int improved = 1;
    while (improved) {
        improved = 0;
        long int      best_pos   = -1;
        long long int best_delta = 0;

        /* Evaluate entire neighborhood to find the steepest ascent */
        for (long int pos = 0; pos < PSize - 1; pos++) {
            long int left_element  = solution[pos];
            long int right_element = solution[pos + 1];
            long long int delta    = CostMat[right_element][left_element]
                                   - CostMat[left_element][right_element];

            if (delta > best_delta) {
                best_delta = delta;
                best_pos   = pos;
            }
        }

        if (best_delta > 0) {
            long int temp              = solution[best_pos];
            solution[best_pos]         = solution[best_pos + 1];
            solution[best_pos + 1]     = temp;
            *current_cost             += best_delta;
            improved                   = 1;
        }
    }
}


//--- Exchange Neighborhood ---

/*
 * Swaps two arbitrary elements at left_pos and right_pos (where left_pos < right_pos).
 * Elements strictly between them do not change relative to each other, but they change
 * relative to the two swapped elements.
 * Delta evaluation complexity: O(n).
 */
static inline long long int compute_exchange_delta(const long int *solution,
                                                   long int left_pos,
                                                   long int right_pos) {
    long int left_element  = solution[left_pos];
    long int right_element = solution[right_pos];

    /* Direct swap impact between the two elements */
    long long int delta    = CostMat[right_element][left_element]
                           - CostMat[left_element][right_element];

    /* Impact of the swapped elements crossing the intermediate elements */
    for (long int middle_pos = left_pos + 1; middle_pos < right_pos; middle_pos++) {
        long int middle_element = solution[middle_pos];
        delta += CostMat[right_element][middle_element] /* new arc: right is now before middle */
               + CostMat[middle_element][left_element]  /* new arc: middle is now before left */
               - CostMat[left_element][middle_element]  /* lost arc: left was before middle */
               - CostMat[middle_element][right_element];/* lost arc: middle was before right */
    }
    return delta;
}


void exchange_first_improvement(long int *solution, long long int *current_cost) {
    int improved = 1;
    while (improved) {
        improved = 0;
        /* The !improved flag breaks both loops natively as soon as a move is applied */
        for (long int left_pos = 0; left_pos < PSize - 1 && !improved; left_pos++) {
            for (long int right_pos = left_pos + 1; right_pos < PSize && !improved; right_pos++) {
                long long int delta = compute_exchange_delta(solution, left_pos, right_pos);
                if (delta > 0) {
                    long int temp          = solution[left_pos];
                    solution[left_pos]     = solution[right_pos];
                    solution[right_pos]    = temp;
                    *current_cost         += delta;
                    improved               = 1;
                }
            }
        }
    }
}

void exchange_best_improvement(long int *solution, long long int *current_cost) {
    int improved = 1;
    while (improved) {
        improved = 0;
        long int      best_left_pos  = -1, best_right_pos = -1;
        long long int best_delta     = 0;

        for (long int left_pos = 0; left_pos < PSize - 1; left_pos++) {
            for (long int right_pos = left_pos + 1; right_pos < PSize; right_pos++) {
                long long int delta = compute_exchange_delta(solution, left_pos, right_pos);
                if (delta > best_delta) {
                    best_delta     = delta;
                    best_left_pos  = left_pos;
                    best_right_pos = right_pos;
                }
            }
        }

        if (best_delta > 0) {
            long int temp                   = solution[best_left_pos];
            solution[best_left_pos]         = solution[best_right_pos];
            solution[best_right_pos]        = temp;
            *current_cost                  += best_delta;
            improved                        = 1;
        }
    }
}


// --- INSERT Neighborhood ---

/*
 * Removes element at from_pos and reinserts it at to_pos.
 * Delta evaluation complexity: O(n).
 */
static inline long long int compute_insert_delta(const long int *solution,
                                                 long int from_pos,
                                                 long int to_pos) {
    long long int delta          = 0;
    long int      moved_element  = solution[from_pos];

    if (from_pos < to_pos) {
        /* Moving right: moved_element shifts AFTER the intermediate block */
        for (long int pos = from_pos + 1; pos <= to_pos; pos++)
            delta += CostMat[solution[pos]][moved_element]
                   - CostMat[moved_element][solution[pos]];
    } else {
        /* Moving left: moved_element shifts BEFORE the intermediate block */
        for (long int pos = to_pos; pos < from_pos; pos++)
            delta += CostMat[moved_element][solution[pos]]
                   - CostMat[solution[pos]][moved_element];
    }
    return delta;
}

/* Shifts the intermediate block of elements to complete the insertion */
static inline void apply_insert(long int *solution, long int from_pos, long int to_pos) {
    long int moved_element = solution[from_pos];
    if (from_pos < to_pos) {
        /* Shift intermediate elements left */
        for (long int pos = from_pos; pos < to_pos; pos++)
            solution[pos] = solution[pos + 1];
    } else {
        /* Shift intermediate elements right */
        for (long int pos = from_pos; pos > to_pos; pos--)
            solution[pos] = solution[pos - 1];
    }
    solution[to_pos] = moved_element;
}

void insert_first_improvement(long int *solution, long long int *current_cost) {
    int improved = 1;
    while (improved) {
        improved = 0;
        for (long int from_pos = 0; from_pos < PSize && !improved; from_pos++) {
            for (long int to_pos = 0; to_pos < PSize && !improved; to_pos++) {
                if (from_pos == to_pos) continue;
                long long int delta = compute_insert_delta(solution, from_pos, to_pos);
                if (delta > 0) {
                    apply_insert(solution, from_pos, to_pos);
                    *current_cost += delta;
                    improved       = 1;
                }
            }
        }
    }
}

void insert_best_improvement(long int *solution, long long int *current_cost) {
    int improved = 1;
    while (improved) {
        improved = 0;
        long int      best_from_pos = -1, best_to_pos = -1;
        long long int best_delta    = 0;

        for (long int from_pos = 0; from_pos < PSize; from_pos++) {
            for (long int to_pos = 0; to_pos < PSize; to_pos++) {
                if (from_pos == to_pos) continue;
                long long int delta = compute_insert_delta(solution, from_pos, to_pos);
                if (delta > best_delta) {
                    best_delta    = delta;
                    best_from_pos = from_pos;
                    best_to_pos   = to_pos;
                }
            }
        }

        if (best_delta > 0) {
            apply_insert(solution, best_from_pos, best_to_pos);
            *current_cost += best_delta;
            improved       = 1;
        }
    }
}


//--- VND ---

/*
 * VND applies first-improvement across a defined sequence of neighborhoods.
 * Rule: On any improvement, reset to the first neighborhood.
 * On failure, advance to the next neighborhood in the sequence.
 * Termination: Local optimum reached across all neighborhoods simultaneously.
 */

// Order: Transpose -> Exchange -> Insert */
void vnd_tei(long int *solution, long long int *current_cost) {
    int neighborhood_index = 0;
    while (neighborhood_index < 3) {
        long long int cost_before = *current_cost;

        if      (neighborhood_index == 0) transpose_first_improvement(solution, current_cost);
        else if (neighborhood_index == 1) exchange_first_improvement (solution, current_cost);
        else                              insert_first_improvement   (solution, current_cost);

        if (*current_cost > cost_before)
            neighborhood_index = 0; //improvement found
        else
            neighborhood_index++;   //no improvement
    }
}

// Order: Transpose -> Insert -> Exchange */
void vnd_tie(long int *solution, long long int *current_cost) {
    int neighborhood_index = 0;
    while (neighborhood_index < 3) {
        long long int cost_before = *current_cost;

        if      (neighborhood_index == 0) transpose_first_improvement(solution, current_cost);
        else if (neighborhood_index == 1) insert_first_improvement   (solution, current_cost);
        else                              exchange_first_improvement  (solution, current_cost);

        if (*current_cost > cost_before)
            neighborhood_index = 0; 
        else
            neighborhood_index++;   
    }
}