# Script: analyse2.R
# Purpose: Statistical analysis of SLS algorithms (ILS and Memetic Algorithm)
#          for Implementation Exercise 2 — Heuristic Optimization (INFO-H-413).
#
# Exercise 2.1 requirements covered:
#   2. Mean RPD from best-known per algorithm (table)
#   3. Correlation plot of RPD values for the SLS algorithms
#   4. Statistical tests (Wilcoxon signed-rank + t-test) on RPD vectors
#   5. RTD: qualified run-time distributions across 25 repetitions
#
# Input CSV format (produced by ./lop_sls):
#   results_main.csv : Instance, Algorithm, Cost, Time_s
#                      (one row = final result of one run)
#   results_rtd.csv  : Instance, Algorithm, Repetition, Cost, Time_s
#                      (one row per improvement event during RTD runs)
#   best_known.csv   : Instance, Best_Known
#                      (optional — max observed cost used as proxy if absent)
#
# Usage:
#   Rscript analyse2.R
#   Rscript analyse2.R results_main.csv results_rtd.csv best_known.csv


# ============================================================
# 0. Configuration
# ============================================================

MAIN_FILE       <- "results_main.csv"
RTD_FILE        <- "results_rtd.csv"
BEST_KNOWN_FILE <- "best_known/best_known.csv"

args <- commandArgs(trailingOnly = TRUE)
if (length(args) >= 1) MAIN_FILE       <- args[1]
if (length(args) >= 2) RTD_FILE        <- args[2]
if (length(args) >= 3) BEST_KNOWN_FILE <- args[3]


RTD_THRESHOLDS <- c(0, 0.1, 0.25, 0.5)



# 1. Load csv
data <- read.csv(MAIN_FILE, sep = ",", check.names = FALSE)
data <- na.omit(data)

cat("=================================================\n")
cat("Main results loaded from:", MAIN_FILE, "\n")
cat("  Rows      :", nrow(data), "\n")
cat("  Algorithms:", paste(sort(unique(data$Algorithm)), collapse = ", "), "\n")
cat("  Instances :", length(unique(data$Instance)), "\n\n")


if (file.exists(BEST_KNOWN_FILE)) {
  bk   <- read.csv(BEST_KNOWN_FILE, sep = ",", check.names = FALSE)
  data <- merge(data, bk, by = "Instance", all.x = TRUE)
  cat("Best-known values loaded from:", BEST_KNOWN_FILE, "\n\n")
} else {
  best_obs <- aggregate(data$Cost,
                        by = list(Instance = data$Instance), FUN = max)
  colnames(best_obs)[2] <- "Best_Known"
  data <- merge(data, best_obs, by = "Instance", all.x = TRUE)
  cat("No best-known file: using max observed cost per instance as proxy.\n\n")
}

# RPD = 100 * (Best_Known - Cost) / Best_Known  [%]
# RPD = 0   means we matched the best-known solution.
# RPD > 0   means our solution is RPD% worse than best-known.
data$Deviation_Pct <- 100.0 * (data$Best_Known - data$Cost) / data$Best_Known


# ============================================================
# 2. Summary table — Avg RPD, SD, Total Time per algorithm
#    (Exercise 2.1, point 2)
# ============================================================

mean_dev  <- aggregate(Deviation_Pct ~ Algorithm, data = data, FUN = mean)
sd_dev    <- aggregate(Deviation_Pct ~ Algorithm, data = data, FUN = sd)
sum_time  <- aggregate(Time_s        ~ Algorithm, data = data, FUN = sum)

colnames(mean_dev)[2]  <- "Avg_Dev_Pct"
colnames(sd_dev)[2]    <- "SD_Dev_Pct"
colnames(sum_time)[2]  <- "Sum_Time_s"

summary_table <- merge(mean_dev, sd_dev,    by = "Algorithm")
summary_table <- merge(summary_table, sum_time, by = "Algorithm")

cat("=================================================\n")
cat("--- Summary Table (1 run per instance, all size-150) ---\n")
print(summary_table)
cat("\n")


# ============================================================
# 3. Prepare RPD vectors for tests and plots
# ============================================================

algorithms <- sort(unique(data$Algorithm))

rpd_wide <- reshape(data[, c("Instance", "Algorithm", "Deviation_Pct")],
                    idvar     = "Instance",
                    timevar   = "Algorithm",
                    direction = "wide")
colnames(rpd_wide) <- gsub("^Deviation_Pct\\.", "", colnames(rpd_wide))
rpd_wide <- rpd_wide[complete.cases(rpd_wide[, algorithms]), ]

ils_rpd <- rpd_wide[["ILS"]]
ma_rpd  <- rpd_wide[["MA"]]

cat("=================================================\n")
cat("Complete instances (both algorithms):", nrow(rpd_wide), "\n\n")


# ============================================================
# 4. Statistical Tests (Exercise 2.1, point 4)
#
# Primary test: Wilcoxon signed-rank (non-parametric, paired).
# RPD distributions are often skewed, so Wilcoxon is preferred
# over the t-test, which is included as a secondary reference.
# ============================================================

cat("=================================================\n")
cat("--- Test 1: ILS vs MA  (Wilcoxon signed-rank, primary) ---\n")
print(wilcox.test(ils_rpd, ma_rpd, paired = TRUE))

cat("=================================================\n")
cat("--- Test 2: ILS vs MA  (paired t-test, secondary) ---\n")
print(t.test(ils_rpd, ma_rpd, paired = TRUE))

cat("=================================================\n")
cat("--- Sign Test: ILS vs MA ---\n")
wins_ILS <- sum(ils_rpd < ma_rpd)
wins_MA  <- sum(ma_rpd  < ils_rpd)
ties     <- sum(ils_rpd == ma_rpd)
cat("  Instances where ILS is better :", wins_ILS, "\n")
cat("  Instances where MA  is better :", wins_MA,  "\n")
cat("  Ties                           :", ties,     "\n")
if ((wins_ILS + wins_MA) > 0) {
  sign_p <- binom.test(c(wins_ILS, wins_MA), alternative = "two.sided")$p.value
  cat("  Sign test p-value:", format.pval(sign_p, digits = 4), "\n\n")
} else {
  cat("  Sign test: all ties — not applicable.\n\n")
}


# ============================================================
# 5. Correlation Plot (Exercise 2.1, point 3)
#    Scatter: ILS RPD vs MA RPD, one point per instance.
#    Points above the diagonal: ILS is better (lower RPD).
#    Points below the diagonal: MA  is better.
# ============================================================

pdf("correlation_plot2.pdf", width = 7, height = 6)

lim_lo <- min(ils_rpd, ma_rpd) * 0.90
lim_hi <- max(ils_rpd, ma_rpd) * 1.10

plot(ils_rpd, ma_rpd,
     xlim = c(lim_lo, lim_hi),
     ylim = c(lim_lo, lim_hi),
     pch  = 21,
     bg   = rgb(0.24, 0.51, 0.95, 0.7),
     col  = "white",
     cex  = 1.3,
     xlab = "ILS  —  RPD from best-known (%)",
     ylab = "MA   —  RPD from best-known (%)",
     main = "Correlation Plot: ILS vs MA\n(one point per size-150 instance)")

abline(a = 0, b = 1, lty = 2, col = "gray40", lwd = 1.5)

r_val <- cor(ils_rpd, ma_rpd, use = "complete.obs")
legend("topleft", legend = paste0("r = ", round(r_val, 3)),
       bty = "n", cex = 1.1)

legend("bottomright",
       legend = c("Above diagonal: ILS better",
                  "Below diagonal: MA  better"),
       bty = "n", cex = 0.85, text.col = "gray30")

dev.off()
cat("Correlation plot saved: correlation_plot2.pdf\n\n")


# ============================================================
# 6. Run-Time Distributions (Exercise 2.1, point 5)
#
#    results_rtd.csv contains one row per improvement event
#    (each time a new best cost was found during an RTD run).
#    For each (instance, algorithm, repetition) we take the
#    LAST row as the final result and keep all intermediate rows
#    to build the empirical CDF of time-to-reach-quality.
#
#    A run is counted as "solved at threshold T" if at any point
#    during the run its best cost satisfied RPD <= T.
#    The solve time is the Time_s of the first such improvement.
# ============================================================

if (!file.exists(RTD_FILE)) {
  cat("RTD file not found:", RTD_FILE, "— skipping RTD plots.\n\n")
} else {

  rtd_raw <- read.csv(RTD_FILE, sep = ",", check.names = FALSE)
  rtd_raw <- na.omit(rtd_raw)

  # Attach best-known
  if (file.exists(BEST_KNOWN_FILE)) {
    bk_rtd  <- read.csv(BEST_KNOWN_FILE, sep = ",", check.names = FALSE)
    rtd_raw <- merge(rtd_raw, bk_rtd, by = "Instance", all.x = TRUE)
  } else {
    best_obs_rtd <- aggregate(rtd_raw$Cost,
                              by = list(Instance = rtd_raw$Instance), FUN = max)
    colnames(best_obs_rtd)[2] <- "Best_Known"
    rtd_raw <- merge(rtd_raw, best_obs_rtd, by = "Instance", all.x = TRUE)
  }
  rtd_raw$Deviation_Pct <- 100.0 * (rtd_raw$Best_Known - rtd_raw$Cost) /
                            rtd_raw$Best_Known

  # First 2 instances (alphabetical order = same as the shell script)
  rtd_instances <- sort(unique(rtd_raw$Instance))[1:min(2, length(unique(rtd_raw$Instance)))]

  n_reps   <- 25
  col_alg  <- c("ILS" = "blue", "MA" = "red")
  lty_map  <- c(1, 2, 3, 4)

  for (inst in rtd_instances) {

    inst_data <- subset(rtd_raw, Instance == inst)
    t_cutoff  <- max(inst_data$Time_s, na.rm = TRUE)

    safe_name <- gsub("[^A-Za-z0-9_]", "_", inst)
    pdf(paste0("rtd_", safe_name, ".pdf"), width = 9, height = 6)

    plot(NULL,
         xlim = c(0, t_cutoff),
         ylim = c(0, 1),
         xlab = "CPU time (seconds)",
         ylab = "P(reach quality target within t)",
         main = paste0("Run-Time Distribution — ", inst,
                       "\n(", n_reps, " repetitions, cutoff = 10 x T_vnd x 500)"),
         las  = 1)
    grid(col = "gray90", lty = 1)
    abline(h = c(0.25, 0.5, 0.75, 1.0), col = "gray80", lty = 3)

    legend_labels <- character(0)
    legend_cols   <- character(0)
    legend_ltys   <- integer(0)

    for (alg in algorithms) {

      alg_data <- subset(inst_data, Algorithm == alg)
      alg_col  <- col_alg[alg]

      for (k in seq_along(RTD_THRESHOLDS)) {

        thresh <- RTD_THRESHOLDS[k]

        # For each repetition, find the FIRST time the threshold was reached
        solve_times <- numeric(0)

        for (rep_id in sort(unique(alg_data$Repetition))) {
          rep_rows <- subset(alg_data, Repetition == rep_id)
          rep_rows <- rep_rows[order(rep_rows$Time_s), ]

          # First row where RPD <= thresh
          hit <- rep_rows[rep_rows$Deviation_Pct <= thresh + 1e-9, ]
          if (nrow(hit) > 0)
            solve_times <- c(solve_times, hit$Time_s[1])
          # Runs that never reached the threshold are simply not counted
          # (their contribution to the CDF remains 0 up to the cutoff)
        }

        if (length(solve_times) == 0) {
          lines(c(0, t_cutoff), c(0, 0), type = "l", col = alg_col, lty = lty_map[k], lwd = 2)
        } else {
          times <- sort(solve_times)
          probs <- seq_along(times) / n_reps
          lines(c(0, times, t_cutoff),
                c(0, probs, max(probs)),
                type = "s", col = alg_col, lty = lty_map[k], lwd = 2)
        }

        legend_labels <- c(legend_labels,
                           paste0(alg, "  RPD \u2264 ", thresh, "%"))
        legend_cols   <- c(legend_cols, alg_col)
        legend_ltys   <- c(legend_ltys, lty_map[k])
      }
    }

    legend("bottomright",
           legend = legend_labels,
           col    = legend_cols,
           lty    = legend_ltys,
           lwd    = 2, bty = "n", cex = 0.9)

    dev.off()
    cat("RTD plot saved: rtd_", safe_name, ".pdf\n", sep = "")
  }
}


# ============================================================
# 7. Final summary
# ============================================================

cat("\n=================================================\n")
cat("ANALYSIS COMPLETE\n")
cat("=================================================\n")
cat("Output files:\n")
cat("  correlation_plot2.pdf     (Exercise 2.1, point 3)\n")
cat("  rtd_<instance>.pdf        (Exercise 2.1, point 5)\n\n")
cat("Mean RPD per algorithm (all size-150 instances, 1 run each):\n")
for (alg in algorithms) {
  m <- mean(data$Deviation_Pct[data$Algorithm == alg], na.rm = TRUE)
  cat(sprintf("  %-6s : %.4f%%\n", alg, m))
}
cat("\n")
