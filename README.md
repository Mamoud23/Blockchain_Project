# Modelisation et simulation d'une attaque par eclipse sur wallet SPV mobile (3G/4G)

Projet final - Master 1 RIST (BDGL-RIST), UFHB, 2025-2026

## Contexte

Ce projet modelise et simule une attaque par eclipse (Heilman et al., 2015) ciblant
un wallet Bitcoin SPV connecte via un reseau mobile 3G/4G. L'objectif est d'evaluer
comment la degradation du signal mobile (handovers, pertes de connexion) cree des
fenetres de vulnerabilite exploitables par un attaquant cherchant a monopoliser les
connexions P2P de la victime.

## Structure du depotBlockchain_Project/
├── src/ # Code NS-3 (wallet SPV, modele de signal)
│ ├── spv-wallet.h/.cc # Application NS-3 : machine a etats + gestion des slots
│ ├── signal-model.h/.cc # Generateur d'evenements de degradation du signal
│ └── main.cc # Scenario principal parametrable
├── scripts/
│ ├── run-all-scenarios.sh # Lance les 4 scenarios x 4 valeurs de r x 5 repetitions
│ └── analyze-results.py # Genere les graphiques a partir du CSV
├── results/
│ ├── resultats.csv # Resultats bruts des 80 simulations
│ ├── taux_eclipse.png
│ ├── fenetre_vulnerabilite.png
│ └── occupation_adverse.png
└── docs/
└── rapport.md # Rapport complet (methodologie, resultats, recommandations)
## Modele

- **K = 8** slots de connexion sortants sur le wallet victime
- **r** noeuds attaquants (variable), tentant de monopoliser les slots en priorite
- **5 etats** du wallet : S0 Connecte, S1 Mobilite, S2 Attente reconnexion,
  S3 Bootstrap, S4 Eclipse (tous les slots controles par l'attaquant)
- Le signal mobile est modelise par un processus stochastique (intervalles
  exponentiels, SINR gaussien) qui declenche les transitions S0 -> S2 -> S3 -> S0
  du wallet, simulant l'effet des handovers et pertes de signal reels.

## Reproduire les resultats

Prerequis : NS-3 compile dans `~/projets/ns-3-dev`, avec le code de `src/` copie
dans `~/projets/ns-3-dev/scratch/eclipse-sim/`.

```bash
cd ~/projets/ns-3-dev && ./ns3 build
~/projets/Blockchain_Project/scripts/run-all-scenarios.sh
python3 ~/projets/Blockchain_Project/scripts/analyze-results.py
```

Voir `docs/rapport.md` pour la methodologie complete, les resultats detailles et
les recommandations.
