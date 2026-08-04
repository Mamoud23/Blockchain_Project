#!/bin/bash
# Lance tous les scenarios comparatifs x valeurs de r x repetitions
# et exporte les resultats dans un seul CSV

BINARY=~/projets/ns-3-dev/build/scratch/eclipse-sim/ns3-dev-main-default
CSV_OUTPUT=~/projets/Blockchain_Project/results/resultats.csv
NUM_REPETITIONS=5
ATTACKER_VALUES="2 4 6 8"

# On repart d'un CSV propre a chaque lancement complet
rm -f "$CSV_OUTPUT"

echo "=== Scenario filaire (reference) ==="
for r in $ATTACKER_VALUES; do
    for run in $(seq 1 $NUM_REPETITIONS); do
        $BINARY --numAttackers=$r --rngRun=$run \
            --scenarioName=filaire --csvOutput="$CSV_OUTPUT" > /dev/null
    done
done

echo "=== Scenario 4G mobilite moderee ==="
for r in $ATTACKER_VALUES; do
    for run in $(seq 1 $NUM_REPETITIONS); do
        $BINARY --numAttackers=$r --rngRun=$run \
            --meanInterval=15 --meanSinr=8 --sinrStdDev=2 --sinrThreshold=5 \
            --scenarioName=4g_modere --csvOutput="$CSV_OUTPUT" > /dev/null
    done
done

echo "=== Scenario 4G mobilite elevee ==="
for r in $ATTACKER_VALUES; do
    for run in $(seq 1 $NUM_REPETITIONS); do
        $BINARY --numAttackers=$r --rngRun=$run \
            --meanInterval=5 --meanSinr=6 --sinrStdDev=3 --sinrThreshold=5 \
            --scenarioName=4g_eleve --csvOutput="$CSV_OUTPUT" > /dev/null
    done
done

echo "=== Scenario 4G degrade (zone rurale) ==="
for r in $ATTACKER_VALUES; do
    for run in $(seq 1 $NUM_REPETITIONS); do
        $BINARY --numAttackers=$r --rngRun=$run \
            --meanInterval=3 --meanSinr=2 --sinrStdDev=4 --sinrThreshold=5 \
            --scenarioName=4g_degrade --csvOutput="$CSV_OUTPUT" > /dev/null
    done
done

echo "Termine. Resultats dans : $CSV_OUTPUT"
wc -l "$CSV_OUTPUT"
