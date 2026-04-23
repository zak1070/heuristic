# Script: analyse.R
# Purpose: Statistical analysis of Iterative Improvement and VND algorithms
#          applied to the Linear Ordering Problem (LOP).
# Output:  Summary tables (Mean Deviation, SD, Total Time) and paired hypothesis 
#          tests (Student's t-test and Wilcoxon signed-rank test) for Ex 1.1 & 1.2.
#
# Command: Rscript analyse.R



# Data Loading and Preprocessing
data <- read.csv("final_data.txt", sep=",", check.names=FALSE)

# Temporary removal of NAs values
data <- na.omit(data)

# 2. Generate summary table using Base R
# Calculate Mean of Percentage Deviation
mean_dev <- aggregate(data$`Deviation_%`, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=mean)
colnames(mean_dev)[3] <- "Avg_Dev_Pct"

sd_dev <- aggregate(data$`Deviation_%`, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=sd)
colnames(sd_dev)[3] <- "SD_Dev_Pct"

# Calculate Total Execution Time (Sum)
sum_time <- aggregate(data$Time_s, by=list(Init=data$Init, Algorithm=data$Algorithm), FUN=sum)
colnames(sum_time)[3] <- "Sum_Time_s"

# Merge aggregated metrics into a unified summary dataframe
summary_table <- merge(mean_dev, sd_dev, by=c("Init", "Algorithm"))
summary_table <- merge(summary_table, sum_time, by=c("Init", "Algorithm"))

print("--- Summary Table for Report ---")
print(summary_table)

# 3. Prepare data for statistical tests (Q1.1)
# Exclude VND algorithms to fairly compare the 6 base algorithms (CW vs Random)
data_q1 <- subset(data, !(Algorithm %in% c("VND_TEI", "VND_TIE")))

cw_data <- subset(data_q1, Init == "CW")$`Deviation_%`
random_data <- subset(data_q1, Init == "Random")$`Deviation_%`

insert_best <- subset(data, Init == "CW" & Algorithm == "Insert_Best")$`Deviation_%`
insert_first <- subset(data, Init == "CW" & Algorithm == "Insert_First")$`Deviation_%`
exchange_first <- subset(data, Init == "CW" & Algorithm == "Exchange_First")$`Deviation_%`


# 4. Statistical Inference for Exercise 1.1
# Executes paired Student's t-test and Wilcoxon tests
print("=====================================================")
print("--- Test 1: CW vs Random Initialization ---")
print(t.test(cw_data, random_data, paired = TRUE))
print(wilcox.test(cw_data, random_data, paired = TRUE))

print("=====================================================")
print("--- Test 2: Insert Best vs Insert First (CW) ---")
print(t.test(insert_best, insert_first, paired = TRUE))
print(wilcox.test(insert_best, insert_first, paired = TRUE))

print("=====================================================")
print("--- Test 3: Insert First vs Exchange First (CW) ---")
print(t.test(insert_first, exchange_first, paired = TRUE))
print(wilcox.test(insert_first, exchange_first, paired = TRUE))

# 5. Data Extraction and Statistical Inference for Exercise 1.2 (VND)
# Vectors for Test 4: VND Ordering Impact
vnd_tei <- subset(data, Init == "CW" & Algorithm == "VND_TEI")$`Deviation_%`
vnd_tie <- subset(data, Init == "CW" & Algorithm == "VND_TIE")$`Deviation_%`

print("=====================================================")
print("--- Test 4: VND_TEI vs VND_TIE (CW) ---")
print(t.test(vnd_tei, vnd_tie, paired = TRUE))
print(wilcox.test(vnd_tei, vnd_tie, paired = TRUE))