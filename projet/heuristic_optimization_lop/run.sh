#!/bin/bash

# Création de l'en-tête (on écrase l'ancien fichier)
echo "Instance,Init,Algorithm,Cost,Time_s" > raw_data.csv

# Lancement du programme C sur toutes les instances
for file in instances/N-*; do
    ./lop --instance "$file" >> raw_data.csv;
done

# Chemins des fichiers
BEST_KNOWN="./best_known/best_known.txt"
INPUT_FILE="raw_data.csv"
OUTPUT_FILE="final_stats.csv"

# Vérification de l'existence du fichier best_known
if [ ! -f "$BEST_KNOWN" ]; then
    echo "Erreur : Le fichier $BEST_KNOWN est introuvable."
    exit 1
fi

# Création de l'en-tête du fichier de sortie avec des virgules
echo "Instance,Init,Algorithm,Cost,Time_s,Deviation_%" > "$OUTPUT_FILE"

# Lecture du fichier d'entrée
while IFS=',' read -r instance init algo cost time_s extra; do
    
    # Ignorer l'en-tête ou les lignes vides
    if [[ "$instance" == "Instance" ]] || [[ -z "$instance" ]]; then
        continue
    fi

    # Extraction du coût optimal dans best_known.txt
    best_cost=$(grep -w "^${instance}" "$BEST_KNOWN" | awk '{print $2}')

    if [ -n "$best_cost" ] && [ "$best_cost" -ne 0 ]; then
        # Calcul de la déviation : ((Best - Cost) / Best) * 100
        deviation=$(awk -v b="$best_cost" -v c="$cost" 'BEGIN { printf "%.6f", ((b - c) / b) * 100 }')
        
        # Écriture dans le fichier de sortie
        echo "${instance},${init},${algo},${cost},${time_s},${deviation}" >> "$OUTPUT_FILE"
    fi
done < "$INPUT_FILE"

echo "Traitement terminé. Résultats dans $OUTPUT_FILE"

# # !/bin/bash
# # Utilise des virgules ici aussi pour l'en-tête
# # echo "Instance,Init,Algorithm,Cost,Time_s" > raw_data.csv
# # for file in instances/N-*; do
# #     ./lop --instance "$file" >> raw_data.csv;
# # done


# # Chemins des fichiers
# BEST_KNOWN="./best_known/best_known.txt"
# INPUT_FILE="raw_data.csv"
# OUTPUT_FILE="final_stats.csv"

# # Vérification de l'existence du fichier best_known
# if [ ! -f "$BEST_KNOWN" ]; then
#     echo "Erreur : Le fichier $BEST_KNOWN est introuvable."
#     exit 1
# fi

# # Création de l'en-tête du fichier de sortie avec des virgules
# echo "Instance,Init,Algorithm,Cost,Time_s,Deviation_%" > "$OUTPUT_FILE"

# # Lecture du fichier d'entrée
# # IFS=',' car le fichier raw_data.csv utilise des virgules
# while IFS=',' read -r instance init algo cost time_s extra; do
    
#     # Ignorer l'en-tête ou les lignes vides
#     if [[ "$instance" == "Instance" ]] || [[ -z "$instance" ]]; then
#         continue
#     fi

#     # Extraction du coût optimal dans best_known.txt
#     best_cost=$(grep -w "^${instance}" "$BEST_KNOWN" | awk '{print $2}')

#     if [ -n "$best_cost" ] && [ "$best_cost" -ne 0 ]; then
#         # Calcul de la déviation : ((Best - Cost) / Best) * 100
#         deviation=$(awk -v b="$best_cost" -v c="$cost" 'BEGIN { printf "%.4f", ((b - c) / b) * 100 }')
#         echo "$instance,$init,$algo,$cost,$time_s,$deviation" >> "$OUTPUT_FILE"
#     else
#         echo "$instance,$init,$algo,$cost,$time_s,N/A" >> "$OUTPUT_FILE"
#     fi

# done < "$INPUT_FILE"

# echo "Traitement terminé. Les données sont disponibles dans $OUTPUT_FILE"