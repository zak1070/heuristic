/* * Heuristic Optimization Assignment — Part 2.
 * Completed by M'hamdi Zakaria, 2026.
 * ---
 * Entry point for the LOP SLS solver (ILS and Memetic Algorithm).
 * * Termination criterion (Exercise 2.1, point 1):
 * The time limit for each run is computed dynamically to ensure 
 * the termination criterion is proportional to instance hardness 
 * and independent of absolute wall-clock speed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include <sys/time.h> /* Added for overall execution timer */

#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "optimization.h"
#include "sls.h"

char *FileName;

static void readOpts(int argc, char **argv, int *repetitions,
                     int *print_main_header, int *print_rtd_header,
                     double *t_vnd_in, long long int *target_cost_in,
                     int *run_vnd_only) {
    int opt;
    FileName           = NULL;
    *repetitions       = 1;
    *print_main_header = 0;
    *print_rtd_header  = 0;
    *t_vnd_in          = 0.0;
    *target_cost_in    = 0;
    *run_vnd_only      = 0; 

    static struct option long_options[] = {
        {"instance",    required_argument, 0, 'i'},
        {"reps",        required_argument, 0, 'r'},
        {"time",        required_argument, 0, 't'},
        {"best",        required_argument, 0, 'b'}, 
        {"header",      no_argument,       0, 'm'},
        {"rtd-header",  no_argument,       0, 'R'},
        {"vnd-only",    no_argument,       0, 'v'}, 
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:r:t:b:mRv", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'i':
            FileName = malloc(strlen(optarg) + 1);
            strcpy(FileName, optarg);
            break;
        case 'r': *repetitions       = atoi(optarg);  break;
        case 't': *t_vnd_in          = atof(optarg);  break;
        case 'm': *print_main_header = 1;             break;
        case 'R': *print_rtd_header  = 1;             break;
        case 'b': *target_cost_in    = atoll(optarg); break;
        case 'v': *run_vnd_only      = 1;             break;
        default:
            fprintf(stderr,
                    "Usage: %s -i <instance> [-r <n>] [-t <time>] [-v] [-m|-R]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (*print_main_header) {
        printf("Instance,Algorithm,Cost,Time_s\n");
        exit(EXIT_SUCCESS);
    }
    if (*print_rtd_header) {
        printf("Instance,Algorithm,Repetition,Cost,Time_s\n");
        exit(EXIT_SUCCESS);
    }
    if (!FileName && !*print_main_header && !*print_rtd_header) {
        fprintf(stderr, "[Error] No instance provided (-i). Exiting.\n");
        exit(EXIT_FAILURE);
    }
}

typedef struct {
    const char *inst_name;
    const char *alg_name;
    int        repetition; 
    int        rtd_mode;  
} RtdCtx;

static void rtd_print(long long int cost, double time_s, void *ctx) {
    RtdCtx *c = (RtdCtx *)ctx;
    if (c->rtd_mode)
        printf("%s,%s,%d,%lld,%.6f\n",
               c->inst_name, c->alg_name, c->repetition, cost, time_s);
}

typedef void (*sls_fn_t)(long int *sol, long long int *cost, double time_limit, 
                         long long int target_cost, rtd_callback_t rtd_cb, void *rtd_ctx);

static void run_main(const char *inst_name, const char *alg_name,
                     sls_fn_t alg,
                     const long int *init_sol, long long int init_cost,
                     double time_limit, long long int target_cost) { 
    long int      *sol  = malloc(PSize * sizeof(long int));
    long long int  cost = init_cost;

    memcpy(sol, init_sol, PSize * sizeof(long int));

    start_timers();
    alg(sol, &cost, time_limit, target_cost, NULL, NULL); 
    double elapsed = elapsed_time(VIRTUAL);

    printf("%s,%s,%lld,%.6f\n", inst_name, alg_name, cost, elapsed);

    free(sol);
}

static void run_rtd(const char *inst_name, const char *alg_name,
                    sls_fn_t alg,
                    const long int *init_sol, long long int init_cost,
                    double time_limit, int repetitions,
                    long int base_seed, long long int target_cost) { 
    for (int rep = 1; rep <= repetitions; rep++) {
        Seed = base_seed + (long int)rep * 12345L;

        long int      *sol  = malloc(PSize * sizeof(long int));
        long long int  cost = init_cost;
        memcpy(sol, init_sol, PSize * sizeof(long int));

        RtdCtx ctx = { inst_name, alg_name, rep, 1 };

        start_timers();
        alg(sol, &cost, time_limit, target_cost, rtd_print, &ctx);
        double elapsed = elapsed_time(VIRTUAL);
        
        printf("%s,%s,%d,%lld,%.6f\n",
               inst_name, alg_name, rep, cost, elapsed);

        free(sol);
    }
}

int main(int argc, char **argv) {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL); /* Initialize overall execution timer */

    int repetitions, print_main_header, print_rtd_header, run_vnd_only; 
    double t_vnd; 
    long long int target_cost;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    readOpts(argc, argv, &repetitions, &print_main_header, &print_rtd_header, &t_vnd, &target_cost, &run_vnd_only);

    if (t_vnd <= 0.0 && !print_main_header && !print_rtd_header && !run_vnd_only) {
        fprintf(stderr, "[Fatal Error] Reference time T_vnd (-t) is missing.\n");
        exit(EXIT_FAILURE);
    }

    CostMat = readInstance(FileName);

    /* Deterministic base seed from instance content */
    long int base_seed = 0L;
    for (long int i = 0; i < PSize; i++)
        for (long int j = 0; j < PSize; j++)
            base_seed += (long int)CostMat[i][j];
    Seed = base_seed;

    char *inst_name = basename(FileName);

    /* ---- Initial solution: CW heuristic ---- */
    long int      *init_sol  = malloc(PSize * sizeof(long int));
    long long int  init_cost;
    createCWSolution(init_sol);
    init_cost = computeCost(init_sol);

    /* ---- Intercept VND only mode ---- */
    if (run_vnd_only) {
        long int *vnd_sol = malloc(PSize * sizeof(long int));
        memcpy(vnd_sol, init_sol, PSize * sizeof(long int));
        long long int vnd_cost = init_cost;

        start_timers();
        vnd_tie(vnd_sol, &vnd_cost); 
        double elapsed = elapsed_time(VIRTUAL);

        /* Print only this specific format for the Bash script parser */
        printf("VND_TIME: %.6f\n", elapsed);

        free(vnd_sol);
        free(init_sol);
        free(FileName);
        exit(EXIT_SUCCESS); 
    }

    double t_main  = t_vnd * 500.0;    
    double t_rtd   = t_vnd * 5000.0;   

    fprintf(stderr, "[INFO] [%s] T_vnd=%.4fs | T_main=%.2fs | T_rtd=%.2fs\n",
            inst_name, t_vnd, t_main, t_rtd);

    if (repetitions == 1) {
        run_main(inst_name, "ILS", ils, init_sol, init_cost, t_main, target_cost);
        run_main(inst_name, "MA",  memetic_algorithm, init_sol, init_cost, t_main, target_cost);
    } else {
        run_rtd(inst_name, "ILS", ils, init_sol, init_cost, t_rtd, repetitions, base_seed, target_cost);
        run_rtd(inst_name, "MA",  memetic_algorithm, init_sol, init_cost, t_rtd, repetitions, base_seed, target_cost);
    }

    free(init_sol);
    free(FileName);

    /* Calculate and output overall execution time to stderr */
    gettimeofday(&end_time, NULL);
    double total_elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                           (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    fprintf(stderr, "[INFO] Process completed in %.4f seconds.\n", total_elapsed);

    return EXIT_SUCCESS;
}
// /* Heuristic Optimization assignment — Part 2.
//    Completed by M'hamdi Zakaria, 2026.
//    ---
//    Entry point for the LOP SLS solver (ILS and Memetic Algorithm).

//    Termination criterion (Exercise 2.1, point 1)
//    -----------------------------------------------
//    The time limit for each run is computed dynamically:
//      T_vnd   = average CPU time of one full VND run on this instance
//      T_main  = T_vnd * 500     (main evaluation, 1 run per instance)
//      T_rtd   = T_vnd * 5000    (RTD evaluation, cutoff = 10 * T_main)

//    T_vnd is measured by running VND once on the CW solution before starting
//    the SLS algorithms.  This ensures the termination criterion is proportional
//    to instance hardness and is independent of absolute wall-clock speed.

//    Output formats
//    --------------
//    results_main.csv  (one run per instance, all size-150 instances):
//      Instance,Algorithm,Cost,Time_s

//    results_rtd.csv   (25 repetitions, first 2 instances only):
//      Instance,Algorithm,Repetition,Cost,Time_s
//      — one row per improvement event (intermediate + final),
//        so that analyse2.R can build proper RTD curves.

//    Usage
//    -----
//    # Print CSV header once, then run all instances:
//    ./lop_sls -m
//    for inst in instances/N-*_150; do
//        ./lop_sls -i "$inst" >> results_main.csv
//    done

//    # RTD (25 reps, cutoff = 10 * T_vnd * 500):
//    ./lop_sls -R
//    ./lop_sls -i instances/N-be75eec_150 -r 25 >> results_rtd.csv
//    ./lop_sls -i instances/N-be75np_150  -r 25 >> results_rtd.csv

//    Flags
//    -----
//    -i <file>   instance path (required for a run)
//    -r <n>      repetitions: 1 = main mode (default), >1 = RTD mode
//    -m          print main CSV header and exit (no -i needed)
//    -R          print RTD  CSV header and exit (no -i needed)
// */

// #include <stdio.h>
// #include <stdlib.h>
// #include <getopt.h>
// #include <string.h>
// #include <libgen.h>

// #include "instance.h"
// #include "utilities.h"
// #include "timer.h"
// #include "optimization.h"
// #include "sls.h"

// char *FileName;



// static void readOpts(int argc, char **argv, int *repetitions,
//                      int *print_main_header, int *print_rtd_header,
//                      double *t_vnd_in, long long int *target_cost_in,
//                      int *run_vnd_only) {
//     int opt;
//     FileName           = NULL;
//     *repetitions       = 1;
//     *print_main_header = 0;
//     *print_rtd_header  = 0;
//     *t_vnd_in          = 0.0;
//     *target_cost_in = 0;
//     *run_vnd_only      = 0;

//     static struct option long_options[] = {
//         {"instance",    required_argument, 0, 'i'},
//         {"reps",        required_argument, 0, 'r'},
//         {"time",        required_argument, 0, 't'},
//         {"best",        required_argument, 0, 'b'}, /* Nouveau flag */
//         {"header",      no_argument,       0, 'm'},
//         {"rtd-header",  no_argument,       0, 'R'},
//         {"vnd-only",    no_argument,       0, 'v'},
//         {0, 0, 0, 0}
//     };

//     int option_index = 0;
//     while ((opt = getopt_long(argc, argv, "i:r:t:b:mR", long_options, &option_index)) != -1) {
//         switch (opt) {
//         case 'i':
//             FileName = malloc(strlen(optarg) + 1);
//             strcpy(FileName, optarg);
//             break;
//         case 'r': *repetitions       = atoi(optarg); break;
//         case 't': *t_vnd_in          = atof(optarg); break; /* Capture du temps */
//         case 'm': *print_main_header = 1;             break;
//         case 'R': *print_rtd_header  = 1;             break;
//         case 'b': *target_cost_in = atoll(optarg); break;
//         case 'v': *run_vnd_only      = 1;            break;
//         default:
//             fprintf(stderr,
//                     "Usage: %s -i <instance> [-r <n>] [-t <time>] [-v] [-m|-R]\n",
//                     argv[0]);
//             exit(1);
//         }
//     }

//     if (*print_main_header) {
//         printf("Instance,Algorithm,Cost,Time_s\n");
//         exit(0);
//     }
//     if (*print_rtd_header) {
//         printf("Instance,Algorithm,Repetition,Cost,Time_s\n");
//         exit(0);
//     }
//     if (!FileName) {
//         fprintf(stderr, "No instance provided (-i). Exiting.\n");
//         exit(1);
//     }
// }

// /* -------------------------------------------------------------------------
//  * RTD callback context: carries all info needed to print a row.
//  * ------------------------------------------------------------------------- */
// typedef struct {
//     const char *inst_name;
//     const char *alg_name;
//     int         repetition; /* 0 in main mode (unused) */
//     int         rtd_mode;   /* 1 = print Repetition column */
// } RtdCtx;

// static void rtd_print(long long int cost, double time_s, void *ctx) {
//     RtdCtx *c = (RtdCtx *)ctx;
//     if (c->rtd_mode)
//         printf("%s,%s,%d,%lld,%.6f\n",
//                c->inst_name, c->alg_name, c->repetition, cost, time_s);
//     /* In main mode we don't use the callback — just kept for uniformity. */
// }

// /* -------------------------------------------------------------------------
//  * SLS function pointer type (with RTD callback)
//  * ------------------------------------------------------------------------- */
// typedef void (*sls_fn_t)(long int *sol, long long int *cost, double time_limit, 
//                          long long int target_cost, rtd_callback_t rtd_cb, void *rtd_ctx);

// /* --- MODIFICATION DE LA SIGNATURE : ajout de long long int target_cost --- */
// static void run_main(const char *inst_name, const char *alg_name,
//                      sls_fn_t alg,
//                      const long int *init_sol, long long int init_cost,
//                      double time_limit, long long int target_cost) { // <--- AJOUT ICI
//     long int      *sol  = malloc(PSize * sizeof(long int));
//     long long int  cost = init_cost;

//     memcpy(sol, init_sol, PSize * sizeof(long int));

//     start_timers();
//     /* On passe target_cost à l'algorithme (ILS ou MA) */
//     alg(sol, &cost, time_limit, target_cost, NULL, NULL); 
//     double elapsed = elapsed_time(VIRTUAL);

//     printf("%s,%s,%lld,%.6f\n", inst_name, alg_name, cost, elapsed);

//     free(sol);
// }

// /* --- MODIFICATION DE LA SIGNATURE : ajout de long long int target_cost --- */
// static void run_rtd(const char *inst_name, const char *alg_name,
//                     sls_fn_t alg,
//                     const long int *init_sol, long long int init_cost,
//                     double time_limit, int repetitions,
//                     long int base_seed, long long int target_cost) { // <--- AJOUT ICI
//     for (int rep = 1; rep <= repetitions; rep++) {
//         Seed = base_seed + (long int)rep * 12345L;

//         long int      *sol  = malloc(PSize * sizeof(long int));
//         long long int  cost = init_cost;
//         memcpy(sol, init_sol, PSize * sizeof(long int));

//         RtdCtx ctx = { inst_name, alg_name, rep, 1 };

//         start_timers();
//         /* On passe target_cost à l'algorithme */
//         alg(sol, &cost, time_limit, target_cost, rtd_print, &ctx);

//         double elapsed = elapsed_time(VIRTUAL);
//         printf("%s,%s,%d,%lld,%.6f\n",
//                inst_name, alg_name, rep, cost, elapsed);

//         free(sol);
//     }
// }


// int main(int argc, char **argv) {
//     int repetitions, print_main_header, print_rtd_header, run_vnd_only; // Ajout ici
//     double t_vnd; 
//     long long int target_cost;

//     setbuf(stdout, NULL);
//     setbuf(stderr, NULL);

//     // On passe l'adresse de run_vnd_only
//     readOpts(argc, argv, &repetitions, &print_main_header, &print_rtd_header, &t_vnd, &target_cost, &run_vnd_only);

//     /* Sécurité : si on n'est pas en mode "header" OU en mode "vnd_only", t_vnd doit être fourni */
//     if (t_vnd <= 0.0 && !print_main_header && !print_rtd_header && !run_vnd_only) {
//         fprintf(stderr, "Erreur fatale: Temps de reference T_vnd (-t) manquant.\n");
//         exit(1);
//     }

//     CostMat = readInstance(FileName);

//     /* Deterministic base seed from instance content */
//     long int base_seed = 0L;
//     for (long int i = 0; i < PSize; i++)
//         for (long int j = 0; j < PSize; j++)
//             base_seed += (long int)CostMat[i][j];
//     Seed = base_seed;

//     char *inst_name = basename(FileName);

//     /* ---- Initial solution: CW heuristic ---- */
//     long int      *init_sol  = malloc(PSize * sizeof(long int));
//     long long int  init_cost;
//     createCWSolution(init_sol);
//     init_cost = computeCost(init_sol);

//     // ==========================================
//     // INTERCEPTION DU FLAG -v
//     // ==========================================
//     if (run_vnd_only) {
//         long int *vnd_sol = malloc(PSize * sizeof(long int));
//         memcpy(vnd_sol, init_sol, PSize * sizeof(long int));
//         long long int vnd_cost = init_cost;

//         start_timers();
//         vnd_tie(vnd_sol, &vnd_cost); // Appel direct
//         double elapsed = elapsed_time(VIRTUAL);

//         // On n'imprime QUE cette ligne pour le Bash
//         printf("VND_TIME: %.6f\n", elapsed);

//         free(vnd_sol);
//         free(init_sol);
//         free(FileName);
//         exit(0); // On stoppe le programme ici !
//     }
//     // ==========================================

//     // Si on arrive ici, c'est qu'on fait un vrai run SLS.
//     double t_main  = t_vnd * 500.0;    /* Exercise 2.1.1 criterion          */
//     double t_rtd   = t_vnd * 5000.0;   /* Exercise 2.1.5: cutoff = 10*T_main */

//     fprintf(stderr, "[%s] T_vnd=%.4fs  T_main=%.2fs  T_rtd=%.2fs\n",
//             inst_name, t_vnd, t_main, t_rtd);

//     if (repetitions == 1) {
//         run_main(inst_name, "ILS", ils, init_sol, init_cost, t_main, target_cost);
//         run_main(inst_name, "MA",  memetic_algorithm, init_sol, init_cost, t_main, target_cost);
//     } else {
//         run_rtd(inst_name, "ILS", ils, init_sol, init_cost, t_rtd, repetitions, base_seed, target_cost);
//         run_rtd(inst_name, "MA",  memetic_algorithm, init_sol, init_cost, t_rtd, repetitions, base_seed, target_cost);
//     }

//     free(init_sol);
//     free(FileName);
//     return 0;
// }
