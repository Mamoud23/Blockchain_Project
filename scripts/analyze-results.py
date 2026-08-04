#!/usr/bin/env python3
"""
Analyse les resultats du CSV et genere les courbes comparatives
pour le rapport (taux d'eclipse, occupation adverse, fenetre de vulnerabilite).
"""
import csv
import os
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")  # pas d'affichage interactif, on sauvegarde direct
import matplotlib.pyplot as plt

CSV_PATH = os.path.expanduser("~/projets/Blockchain_Project/results/resultats.csv")
OUTPUT_DIR = os.path.expanduser("~/projets/Blockchain_Project/results")

SCENARIO_LABELS = {
    "filaire": "Filaire (reference)",
    "4g_modere": "4G mobilite moderee",
    "4g_eleve": "4G mobilite elevee",
    "4g_degrade": "4G degrade (rural)",
}
SCENARIO_ORDER = ["filaire", "4g_modere", "4g_eleve", "4g_degrade"]
COLORS = {"filaire": "tab:blue", "4g_modere": "tab:green",
          "4g_eleve": "tab:orange", "4g_degrade": "tab:red"}

# --- Chargement et agregation des donnees ---
data = defaultdict(lambda: defaultdict(list))  # data[scenario][numAttackers] = [rows]

with open(CSV_PATH, newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        scenario = row["scenario"]
        r = int(row["numAttackers"])
        data[scenario][r].append(row)

def average(rows, key):
    vals = [float(row[key]) for row in rows]
    return sum(vals) / len(vals)

# --- Graphique 1 : Taux d'eclipse moyen selon r, par scenario ---
plt.figure(figsize=(8, 5))
for scenario in SCENARIO_ORDER:
    r_values = sorted(data[scenario].keys())
    eclipse_rates = [average(data[scenario][r], "eclipseCount") for r in r_values]
    plt.plot(r_values, eclipse_rates, marker="o", label=SCENARIO_LABELS[scenario],
              color=COLORS[scenario])
plt.xlabel("Nombre de noeuds attaquants (r)")
plt.ylabel("Taux moyen d'eclipses detectees")
plt.title("Taux d'eclipse selon le nombre d'attaquants et le scenario reseau")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, "taux_eclipse.png"), dpi=150)
plt.close()

# --- Graphique 2 : Fenetre de vulnerabilite (temps S2+S3) selon r, par scenario ---
plt.figure(figsize=(8, 5))
for scenario in SCENARIO_ORDER:
    r_values = sorted(data[scenario].keys())
    vuln_times = []
    for r in r_values:
        rows = data[scenario][r]
        avg_s2 = average(rows, "timeS2")
        avg_s3 = average(rows, "timeS3")
        vuln_times.append(avg_s2 + avg_s3)
    plt.plot(r_values, vuln_times, marker="o", label=SCENARIO_LABELS[scenario],
              color=COLORS[scenario])
plt.xlabel("Nombre de noeuds attaquants (r)")
plt.ylabel("Temps moyen en fenetre de vulnerabilite S2+S3 (s)")
plt.title("Duree de la fenetre de vulnerabilite selon le scenario reseau")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, "fenetre_vulnerabilite.png"), dpi=150)
plt.close()

# --- Graphique 3 : Taux d'occupation adverse moyen selon r, par scenario ---
plt.figure(figsize=(8, 5))
for scenario in SCENARIO_ORDER:
    r_values = sorted(data[scenario].keys())
    occ_rates = [average(data[scenario][r], "occupancyRate") for r in r_values]
    plt.plot(r_values, occ_rates, marker="o", label=SCENARIO_LABELS[scenario],
              color=COLORS[scenario])
plt.xlabel("Nombre de noeuds attaquants (r)")
plt.ylabel("Taux moyen d'occupation adverse des slots")
plt.title("Occupation adverse des slots selon le scenario reseau")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, "occupation_adverse.png"), dpi=150)
plt.close()

print("Graphiques generes dans :", OUTPUT_DIR)
print(" - taux_eclipse.png")
print(" - fenetre_vulnerabilite.png")
print(" - occupation_adverse.png")

# --- Tableau recapitulatif texte ---
print("\n=== Tableau recapitulatif (r=8, cas le plus severe) ===")
for scenario in SCENARIO_ORDER:
    if 8 in data[scenario]:
        rows = data[scenario][8]
        print(f"{SCENARIO_LABELS[scenario]:30s} | "
              f"Eclipse: {average(rows, 'eclipseCount'):.2f} | "
              f"S2+S3: {average(rows, 'timeS2') + average(rows, 'timeS3'):.1f}s | "
              f"Occupation: {average(rows, 'occupancyRate'):.2f}")
