/*  Heuristic Optimization assignment, 2026.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto.
    Completed by M'hamdi Zakaria, Computer Science student at ULB.
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/**
 * @file sls.c
 * @brief Implementation of ILS and Memetic Algorithm for the LOP.
 *
 * Both algorithms use insert_first_improvement as their core local search,
 * which is the fastest and most effective procedure per Schiavinotto & Stützle
 * (2004, Table I).  The ILS perturbation uses random interchange (exchange of
 * two arbitrary positions), which cannot be undone by a single insert move,
 * ensuring sufficient diversification.  The MA uses the Order-Based (OB)
 * crossover operator, identified as one of the two best crossover operators
 * for the LOP in the same reference.
 *
 * RTD support
 * -----------
 * Both algorithms accept an optional improvement callback (rtd_cb) and a
 * context pointer (rtd_ctx).  When non-NULL, the callback is invoked every
 * time a new best solution is found, passing the current cost and elapsed
 * time.  main2.c uses this to print intermediate RTD rows without any change
 * to the algorithm logic itself.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "optimization.h"
#include "sls.h"

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

/**
 * @brief Applies k random interchange moves to a solution (perturbation).
 *
 * An interchange move swaps two elements at positions i and j (i != j).
 * This is the perturbation operator used by Schiavinotto & Stützle (2004)
 * because it cannot be undone by insert moves in a single step, ensuring
 * the perturbed solution is in a genuinely different basin of attraction.
 *
 * The cost is NOT updated incrementally here; the caller must recompute
 * or pass cost through the local search that follows.
 *
 * @param solution  Permutation array (modified in-place).
 * @param k         Number of interchange moves to apply.
 */
static void apply_random_interchanges(long int *solution, int k) {
    for (int move = 0; move < k; move++) {
        long int pos_a = (long int)(ran01(&Seed) * PSize);
        long int pos_b;
        do {
            pos_b = (long int)(ran01(&Seed) * PSize);
        } while (pos_b == pos_a);

        long int tmp        = solution[pos_a];
        solution[pos_a]     = solution[pos_b];
        solution[pos_b]     = tmp;
    }
}

/* =========================================================================
 * ILS — Iterated Local Search
 * ========================================================================= */

void ils(long int *sol, long long int *cost, double time_limit,
         long long int target_cost, rtd_callback_t rtd_cb, void *rtd_ctx) { 

    long int      *best_sol     = malloc(PSize * sizeof(long int));
    long int      *current_sol  = malloc(PSize * sizeof(long int));
    long long int  best_cost, current_cost;

    /* Step 1: local search on the provided initial solution */
    memcpy(current_sol, sol, PSize * sizeof(long int));
    current_cost = *cost;
    insert_first_improvement(current_sol, &current_cost);

    memcpy(best_sol, current_sol, PSize * sizeof(long int));
    best_cost = current_cost;

    /* Report initial local optimum */
    if (rtd_cb)
        rtd_cb(best_cost, elapsed_time(VIRTUAL), rtd_ctx);

    /* Main ILS loop */
    while (elapsed_time(VIRTUAL) < time_limit) {
        
        if (target_cost > 0 && best_cost >= target_cost) break;

        /* Step 2: Perturbation — ILS_PERTURBATION_STRENGTH random interchanges */
        memcpy(current_sol, best_sol, PSize * sizeof(long int));
        current_cost = best_cost;
        apply_random_interchanges(current_sol, ILS_PERTURBATION_STRENGTH);
        current_cost = computeCost(current_sol); /* recompute after perturbation */

        /* Step 3: Local search on perturbed solution */
        insert_first_improvement(current_sol, &current_cost);

        /* Step 4: Acceptance criterion — accept small worsening (Schiavinotto 2004) */
        if (current_cost >= (long long int)(best_cost * (1.0 - ILS_ACCEPT_EPSILON))) {
            if (current_cost > best_cost) {
                memcpy(best_sol, current_sol, PSize * sizeof(long int));
                best_cost = current_cost;

                /* RTD: report every improvement */
                if (rtd_cb)
                    rtd_cb(best_cost, elapsed_time(VIRTUAL), rtd_ctx);
            }
        }
    }

    /* Return best found */
    memcpy(sol, best_sol, PSize * sizeof(long int));
    *cost = best_cost;

    free(best_sol);
    free(current_sol);
}

/* =========================================================================
 * Memetic Algorithm — population management helpers
 * ========================================================================= */

/** A population individual: permutation + its cost. */
typedef struct {
    long int      *perm;
    long long int  cost;
} Individual;

/** Allocate a fresh individual (permutation buffer only, cost undefined). */
static Individual *alloc_individual(void) {
    Individual *ind = malloc(sizeof(Individual));
    ind->perm = malloc(PSize * sizeof(long int));
    return ind;
}

/** Deep-copy src into dst (must be already allocated). */
static void copy_individual(Individual *dst, const Individual *src) {
    memcpy(dst->perm, src->perm, PSize * sizeof(long int));
    dst->cost = src->cost;
}

/** Free an individual and its permutation buffer. */
static void free_individual(Individual *ind) {
    free(ind->perm);
    free(ind);
}

/**
 * @brief Comparator for qsort: descending order by cost (best first).
 */
static int cmp_individual_desc(const void *a, const void *b) {
    const Individual *ia = *(const Individual **)a;
    const Individual *ib = *(const Individual **)b;
    if (ia->cost > ib->cost) return -1;
    if (ia->cost < ib->cost) return  1;
    return 0;
}

/**
 * @brief Order-Based (OB) crossover operator.
 *
 * Algorithm (Syswerda 1990, used in Schiavinotto & Stützle 2004):
 *   1. Randomly select k positions from parent1.
 *   2. Copy parent2 into offspring.
 *   3. Re-order the elements occupying those k positions in offspring
 *      according to the relative order they appear in parent1.
 *
 * The intuition: the offspring inherits the global ordering structure of
 * parent2, but corrects a random subset of k positions to follow parent1's
 * preference ordering — preserving partial ordering information from both
 * parents without introducing duplicates.
 *
 * @param parent1   First parent permutation.
 * @param parent2   Second parent permutation.
 * @param offspring Output permutation (must be pre-allocated, size PSize).
 * @param k         Number of positions to re-order (typically PSize/3).
 */
static void ob_crossover(const long int *parent1,
                          const long int *parent2,
                          long int       *offspring,
                          int             k) {
    /* Step 1: Choose k distinct random positions */
    char *selected = calloc(PSize, sizeof(char));
    int   chosen   = 0;
    while (chosen < k) {
        long int pos = (long int)(ran01(&Seed) * PSize);
        if (!selected[pos]) {
            selected[pos] = 1;
            chosen++;
        }
    }

    /* Step 2: Copy parent2 into offspring as base */
    memcpy(offspring, parent2, PSize * sizeof(long int));

    /* Mark which elements are sitting in selected positions of offspring */
    char *in_selected_pos = calloc(PSize, sizeof(char));
    for (long int pos = 0; pos < PSize; pos++)
        if (selected[pos])
            in_selected_pos[offspring[pos]] = 1;

    /* Walk parent1 and collect those elements in parent1's order */
    long int *ordered_elements = malloc(k * sizeof(long int));
    int       idx              = 0;
    for (long int pos = 0; pos < PSize; pos++)
        if (in_selected_pos[parent1[pos]])
            ordered_elements[idx++] = parent1[pos];

    /* Place them back into offspring at the selected positions */
    idx = 0;
    for (long int pos = 0; pos < PSize; pos++)
        if (selected[pos])
            offspring[pos] = ordered_elements[idx++];

    free(selected);
    free(in_selected_pos);
    free(ordered_elements);
}

/* =========================================================================
 * Memetic Algorithm — main procedure
 * ========================================================================= */

void memetic_algorithm(long int *sol, long long int *cost, double time_limit,
                       long long int target_cost, rtd_callback_t rtd_cb, void *rtd_ctx) { 

    int pop_size     = MA_POPULATION_SIZE;
    int offspring_n  = MA_OFFSPRING_PER_GEN;
    int total_slots  = pop_size + offspring_n;
    int ob_k         = PSize / 3;
    if (ob_k < 1) ob_k = 1;

    Individual **pool = malloc(total_slots * sizeof(Individual *));
    for (int i = 0; i < total_slots; i++)
        pool[i] = alloc_individual();

    Individual *best_ever = alloc_individual();

    /* ---- Initialization ---- */
    createCWSolution(pool[0]->perm);
    pool[0]->cost = computeCost(pool[0]->perm);
    insert_first_improvement(pool[0]->perm, &pool[0]->cost);

    for (int i = 1; i < pop_size; i++) {
        createRandomSolution(pool[i]->perm);
        pool[i]->cost = computeCost(pool[i]->perm);
        insert_first_improvement(pool[i]->perm, &pool[i]->cost);
    }

    qsort(pool, pop_size, sizeof(Individual *), cmp_individual_desc);
    copy_individual(best_ever, pool[0]);

    /* Report initial best */
    if (rtd_cb)
        rtd_cb(best_ever->cost, elapsed_time(VIRTUAL), rtd_ctx);

    int stagnation_counter = 0;

    /* ---- Main generational loop ---- */
    while (elapsed_time(VIRTUAL) < time_limit) {
        /* EARLY STOPPING: Arrêt si l'objectif est atteint */
        if (target_cost > 0 && best_ever->cost >= target_cost) break;

        long long int prev_best = best_ever->cost;

        /* --- Crossover: generate offspring --- */
        for (int o = 0; o < offspring_n; o++) {
            int pa = (int)(ran01(&Seed) * pop_size);
            int pb;
            do {
                pb = (int)(ran01(&Seed) * pop_size);
            } while (pb == pa);

            ob_crossover(pool[pa]->perm, pool[pb]->perm,
                         pool[pop_size + o]->perm, ob_k);

            pool[pop_size + o]->cost = computeCost(pool[pop_size + o]->perm);
            insert_first_improvement(pool[pop_size + o]->perm,
                                     &pool[pop_size + o]->cost);

            /* CORRECTION ICI : On coupe immédiatement si le temps est écoulé 
               ou si l'enfant qu'on vient de créer a atteint l'objectif cible */
            if (elapsed_time(VIRTUAL) > time_limit || 
               (target_cost > 0 && pool[pop_size + o]->cost >= target_cost)) {
                break;
            }
        }

        /* --- Selection: keep best pop_size individuals from merged pool --- */
        qsort(pool, total_slots, sizeof(Individual *), cmp_individual_desc);

        /* Update best_ever */
        if (pool[0]->cost > best_ever->cost) {
            copy_individual(best_ever, pool[0]);

            /* RTD: report every improvement */
            if (rtd_cb)
                rtd_cb(best_ever->cost, elapsed_time(VIRTUAL), rtd_ctx);
        }

        /* --- Diversification: partial restart if stagnating --- */
        if (best_ever->cost > prev_best)
            stagnation_counter = 0;
        else
            stagnation_counter++;

        if (stagnation_counter >= MA_DIVERSIFICATION_LIMIT) {
            copy_individual(pool[0], best_ever);
            for (int i = 1; i < pop_size; i++) {
                createRandomSolution(pool[i]->perm);
                pool[i]->cost = computeCost(pool[i]->perm);
                insert_first_improvement(pool[i]->perm, &pool[i]->cost);
            }
            stagnation_counter = 0;
        }
    }

    /* Return best solution ever found */
    memcpy(sol, best_ever->perm, PSize * sizeof(long int));
    *cost = best_ever->cost;

    free_individual(best_ever);
    for (int i = 0; i < total_slots; i++)
        free_individual(pool[i]);
    free(pool);
}
