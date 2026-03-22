# 1. Load data
# Disable name checking to keep "Deviation_%" intact
data <- read.csv("final_stats.csv", sep=",", check.names=FALSE)

# Temporary removal of NAs (ensure best_known.txt is fixed)
data <- na.omit(data)

# 2. Generate summary table using Base R
# Mean
mean_dev <- aggregate(data$`Deviation_%`, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=mean)
colnames(mean_dev)[3] <- "Avg_Dev_Pct"

# Standard Deviation
sd_dev <- aggregate(data$`Deviation_%`, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=sd)
colnames(sd_dev)[3] <- "SD_Dev_Pct"

# Sum of times
sum_time <- aggregate(data$Time_s, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=sum)
colnames(sum_time)[3] <- "Sum_Time_s"

# Merge
summary_table <- merge(mean_dev, sd_dev, by=c("Init", "Algorithm"))
summary_table <- merge(summary_table, sum_time, by=c("Init", "Algorithm"))

print("--- Summary Table for Report ---")
print(summary_table)

# 3. Prepare data for statistical tests (Q1.1)
# On exclut les VND pour comparer équitablement les 6 algos de base (CW vs Random)
data_q1 <- subset(data, !(Algorithm %in% c("VND_TEI", "VND_TIE")))

cw_data <- subset(data_q1, Init == "CW")$`Deviation_%`
random_data <- subset(data_q1, Init == "Random")$`Deviation_%`

insert_best <- subset(data, Init == "CW" & Algorithm == "Insert_Best")$`Deviation_%`
insert_first <- subset(data, Init == "CW" & Algorithm == "Insert_First")$`Deviation_%`

# 4. Execute paired Student's t-test and Wilcoxon tests (Q1.1)
print("=====================================================")
print("--- Test 1: CW vs Random Initialization ---")
print(t.test(cw_data, random_data, paired = TRUE))
print(wilcox.test(cw_data, random_data, paired = TRUE))

print("=====================================================")
print("--- Test 2: Insert Best vs Insert First (CW) ---")
print(t.test(insert_best, insert_first, paired = TRUE))
print(wilcox.test(insert_best, insert_first, paired = TRUE))

# 5. Prepare data and execute statistical tests for VND (Q1.2)
vnd_tei <- subset(data, Init == "CW" & Algorithm == "VND_TEI")$`Deviation_%`
vnd_tie <- subset(data, Init == "CW" & Algorithm == "VND_TIE")$`Deviation_%`

print("=====================================================")
print("--- Test 3: VND_TEI vs VND_TIE (CW) ---")
print(t.test(vnd_tei, vnd_tie, paired = TRUE))
print(wilcox.test(vnd_tei, vnd_tie, paired = TRUE))

# # 1. Load data
# # Disable name checking to keep "Deviation_%" intact
# data <- read.csv("final_stats.csv", sep=",", check.names=FALSE)

# # Temporary removal of NAs (ensure best_known.txt is fixed)
# data <- na.omit(data)

# # 2. Generate summary table using Base R
# # Mean
# mean_dev <- aggregate(data$`Deviation_%`, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=mean)
# colnames(mean_dev)[3] <- "Avg_Dev_Pct"

# # Standard Deviation
# sd_dev <- aggregate(data$`Deviation_%`, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=sd)
# colnames(sd_dev)[3] <- "SD_Dev_Pct"

# # Sum of times
# sum_time <- aggregate(data$Time_s, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=sum)
# colnames(sum_time)[3] <- "Sum_Time_s"

# # Merge
# summary_table <- merge(mean_dev, sd_dev, by=c("Init", "Algorithm"))
# summary_table <- merge(summary_table, sum_time, by=c("Init", "Algorithm"))

# print("--- Summary Table for Report ---")
# print(summary_table)

# # 3. Prepare data for statistical tests (paired)
# cw_data <- subset(data, Init == "CW")$`Deviation_%`
# random_data <- subset(data, Init == "Random")$`Deviation_%`

# insert_best <- subset(data, Init == "CW" & Algorithm == "Insert_Best")$`Deviation_%`
# insert_first <- subset(data, Init == "CW" & Algorithm == "Insert_First")$`Deviation_%`

# # 4. Execute paired Student's t-test and Wilcoxon tests
# print("=====================================================")
# print("--- Test 1: CW vs Random Initialization ---")
# print(t.test(cw_data, random_data, paired = TRUE))
# print(wilcox.test(cw_data, random_data, paired = TRUE))

# print("=====================================================")
# print("--- Test 2: Insert Best vs Insert First (CW) ---")
# print(t.test(insert_best, insert_first, paired = TRUE))
# print(wilcox.test(insert_best, insert_first, paired = TRUE))