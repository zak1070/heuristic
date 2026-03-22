/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Lcaol Search Algorithm for Linear Ordering Problem
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
#include <values.h>

#include "optimization.h" 
#include "instance.h"
#include "utilities.h"

#ifdef __MINGW32__
#include <float.h>
#define MAX_FLOAT FLT_MAX
#else
#define MAX_FLOAT MAXFLOAT
#endif


long int **CostMat;


long long int computeCost (long int *s ) {
    int h,k;
    long long int sum;
    
    /* Diagonal value are not considered */
    for (sum = 0, h = 0; h < PSize; h++ ) 
	for ( k = h + 1; k < PSize; k++ )
	    sum += CostMat[s[h]][s[k]];
    return(sum);
}


void createRandomSolution(long int *s) {
    int j; 
    long int *random;

    random = generate_random_vector(PSize);
    for ( j = 0 ; j < PSize ; j++ ) {
      s[j] = random[j];
    }
    free ( random );
}



typedef struct {
    long int id;
    long long int score;
} CWItem;

int compare_cw(const void *a, const void *b) {
    long long int scoreA = ((CWItem*)a)->score;
    long long int scoreB = ((CWItem*)b)->score;
    if (scoreA < scoreB) return 1;
    if (scoreA > scoreB) return -1;
    return 0;
}

void createCWSolution(long int *s) {
    CWItem *items = malloc(PSize * sizeof(CWItem));
    if (!items) fatal("Erreur allocation CWItem");

    for (long int i = 0; i < PSize; i++) {
        items[i].id = i;
        items[i].score = 0;
        for (long int j = 0; j < PSize; j++) {
            if (i != j) {
                items[i].score += CostMat[i][j]; 
                items[i].score -= CostMat[j][i]; 
            }
        }
    }

    qsort(items, PSize, sizeof(CWItem), compare_cw);

    for (long int i = 0; i < PSize; i++) {
        s[i] = items[i].id;
    }
    free(items);
}

void transpose_first_improvement(long int *s, long long int *current_cost) {
    int improvement = 1;
    while (improvement) {
        improvement = 0;
        for (long int i = 0; i < PSize - 1; i++) {
            long int p1 = s[i];
            long int p2 = s[i+1];
            // Delta-évaluation O(1)
            long long int delta = CostMat[p2][p1] - CostMat[p1][p2];

            if (delta > 0) {
                s[i] = p2;
                s[i+1] = p1;
                *current_cost += delta;
                improvement = 1;
                break; // First-improvement : on redémarre au premier gain
            }
        }
    }
}

void transpose_best_improvement(long int *s, long long int *current_cost) {
    int improvement = 1;
    while (improvement) {
        improvement = 0;
        long int best_i = -1;
        long long int max_delta = 0;

        for (long int i = 0; i < PSize - 1; i++) {
            long int p1 = s[i];
            long int p2 = s[i+1];
            long long int delta = CostMat[p2][p1] - CostMat[p1][p2];

            if (delta > max_delta) {
                max_delta = delta;
                best_i = i;
            }
        }

        if (max_delta > 0) {
            long int temp = s[best_i];
            s[best_i] = s[best_i+1];
            s[best_i+1] = temp;
            *current_cost += max_delta;
            improvement = 1; // Best-improvement : on applique le meilleur gain
        }
    }
}

// Delta-évaluation optimisée pour Exchange
static inline long long int compute_exchange_delta(const long int *s, long int i, long int j) {
    long int pi = s[i];
    long int pj = s[j];
    long long int delta = CostMat[pj][pi] - CostMat[pi][pj];
    
    for (long int k = i + 1; k < j; k++) {
        long int pk = s[k];
        delta += CostMat[pj][pk] + CostMat[pk][pi] - CostMat[pi][pk] - CostMat[pk][pj];
    }
    return delta;
}

void exchange_first_improvement(long int *s, long long int *current_cost) {
    int improvement = 1;
    while (improvement) {
        improvement = 0;
        for (long int i = 0; i < PSize - 1; i++) {
            for (long int j = i + 1; j < PSize; j++) {
                long long int delta = compute_exchange_delta(s, i, j);
                if (delta > 0) {
                    long int temp = s[i];
                    s[i] = s[j];
                    s[j] = temp;
                    *current_cost += delta;
                    improvement = 1;
                    // TODO eviter goto
                    goto next_iteration_first; // Cassure stricte de la double boucle
                }
            }
        }
        next_iteration_first:;
    }
}

void exchange_best_improvement(long int *s, long long int *current_cost) {
    int improvement = 1;
    while (improvement) {
        improvement = 0;
        long int best_i = -1, best_j = -1;
        long long int max_delta = 0;

        for (long int i = 0; i < PSize - 1; i++) {
            for (long int j = i + 1; j < PSize; j++) {
                long long int delta = compute_exchange_delta(s, i, j);
                if (delta > max_delta) {
                    max_delta = delta;
                    best_i = i;
                    best_j = j;
                }
            }
        }

        if (max_delta > 0) {
            long int temp = s[best_i];
            s[best_i] = s[best_j];
            s[best_j] = temp;
            *current_cost += max_delta;
            improvement = 1;
        }
    }
}
// Delta-évaluation optimisée pour Insert
static inline long long int compute_insert_delta(const long int *s, long int i, long int j) {
    long long int delta = 0;
    long int pi = s[i];
    
    if (i < j) {
        // Déplacement vers la droite : pi passe après les éléments k
        for (long int k = i + 1; k <= j; k++) {
            delta += CostMat[s[k]][pi] - CostMat[pi][s[k]];
        }
    } else {
        // Déplacement vers la gauche : pi passe avant les éléments k
        for (long int k = j; k < i; k++) {
            delta += CostMat[pi][s[k]] - CostMat[s[k]][pi];
        }
    }
    return delta;
}

// Application du mouvement Insert
static inline void apply_insert( long int *s, long int i, long int j) {
    long int temp = s[i];
    if (i < j) {
        for (long int k = i; k < j; k++) s[k] = s[k+1];
    } else {
        for (long int k = i; k > j; k--) s[k] = s[k-1];
    }
    s[j] = temp;
}

void insert_first_improvement(long int *s, long long int *current_cost) {
    int improvement = 1;
    while (improvement) {
        improvement = 0;
        for (long int i = 0; i < PSize; i++) {
            for (long int j = 0; j < PSize; j++) {
                if (i == j) continue;
                long long int delta = compute_insert_delta(s, i, j);
                if (delta > 0) {
                    apply_insert(s, i, j);
                    *current_cost += delta;
                    improvement = 1;
                    // TODO eviter goto
                    goto next_iteration_first_ins;
                }
            }
        }
        next_iteration_first_ins:;
    }
}

void insert_best_improvement(long int *s, long long int *current_cost) {
    int improvement = 1;
    while (improvement) {
        improvement = 0;
        long int best_i = -1, best_j = -1;
        long long int max_delta = 0;

        for (long int i = 0; i < PSize; i++) {
            for (long int j = 0; j < PSize; j++) {
                if (i == j) continue;
                long long int delta = compute_insert_delta(s, i, j);
                if (delta > max_delta) {
                    max_delta = delta;
                    best_i = i;
                    best_j = j;
                }
            }
        }

        if (max_delta > 0) {
            apply_insert(s, best_i, best_j);
            *current_cost += max_delta;
            improvement = 1;
        }
    }
}

// VND 1 : Transpose -> Exchange -> Insert
void vnd_tei(long int *s, long long int *current_cost) {
    int k = 0;
    while (k < 3) {
        long long int old_cost = *current_cost;
        
        if (k == 0) transpose_first_improvement(s, current_cost);
        else if (k == 1) exchange_first_improvement(s, current_cost);
        else if (k == 2) insert_first_improvement(s, current_cost);
        
        if (*current_cost > old_cost) k = 0;
        else k++;
    }
}

// VND 2 : Transpose -> Insert -> Exchange
void vnd_tie(long int *s, long long int *current_cost) {
    int k = 0;
    while (k < 3) {
        long long int old_cost = *current_cost;
        
        if (k == 0) transpose_first_improvement(s, current_cost);
        else if (k == 1) insert_first_improvement(s, current_cost);
        else if (k == 2) exchange_first_improvement(s, current_cost);
        
        if (*current_cost > old_cost) k = 0;
        else k++;
    }
}