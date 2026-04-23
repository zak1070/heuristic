/*  Heuristic Optimization assignment, 2026.
    Completed by M'hamdi Zakaria, Computer Science student at ULB.
*/

/**
 * @file sls.h
 * @brief Public interface for ILS and Memetic Algorithm (LOP, Part 2).
 *
 * Algorithm parameters
 * --------------------
 * ILS_PERTURBATION_STRENGTH : number of random interchanges per perturbation.
 *   Set to 4 following Schiavinotto & Stützle (2004), who report 3–5 moves
 *   to be effective for n=150 instances.
 *
 * ILS_ACCEPT_EPSILON : relative tolerance for the acceptance criterion.
 *   A candidate solution is accepted if its cost >= best * (1 - epsilon).
 *   0.02 (2%) allows mild hill-climbing escapes without excessive degradation.
 *
 * MA_POPULATION_SIZE : number of individuals kept between generations.
 *   10 balances diversity against per-generation overhead for n=150.
 *
 * MA_OFFSPRING_PER_GEN : offspring produced each generation via OB crossover.
 *   Equal to pop_size so the merged pool doubles before truncation selection.
 *
 * MA_DIVERSIFICATION_LIMIT : generations without improvement before a partial
 *   population restart (keep best, regenerate rest randomly).
 *   20 generations is conservative enough to let crossover exploit good building
 *   blocks before forcing diversification.
 *
 * RTD callback
 * ------------
 * Both algorithms accept an optional rtd_callback_t.  When non-NULL it is
 * called each time a new best solution is found, with the current cost and
 * elapsed wall-clock seconds.  Pass NULL for normal (non-RTD) runs.
 */

#ifndef SLS_H
#define SLS_H

// ---- Algorithm parameters
#define ILS_PERTURBATION_STRENGTH  8
#define ILS_ACCEPT_EPSILON         0.0001

#define MA_POPULATION_SIZE         20
#define MA_OFFSPRING_PER_GEN       20
#define MA_DIVERSIFICATION_LIMIT   20

/* ---- RTD callback type ---- */
/**
 * @brief Called by ILS / MA whenever a new best cost is found.
 *
 * @param cost     Current best cost.
 * @param time_s   Elapsed CPU time in seconds (from start_timers()).
 * @param ctx      Opaque context pointer supplied by the caller.
 */
typedef void (*rtd_callback_t)(long long int cost, double time_s, void *ctx);



void ils(long int *sol, long long int *cost, double time_limit,
         long long int target_cost, rtd_callback_t rtd_cb, void *rtd_ctx);

void memetic_algorithm(long int *sol, long long int *cost, double time_limit,
                       long long int target_cost, rtd_callback_t rtd_cb, void *rtd_ctx);

#endif /* SLS_H */
