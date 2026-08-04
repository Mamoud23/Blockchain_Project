# Rapport - Attaque par eclipse sur wallet SPV mobile (3G/4G)

## 1. Problematique et question de recherche

Un wallet SPV maintient un nombre limite de connexions P2P sortantes (K=8 par
defaut dans Bitcoin Core). Une attaque par eclipse consiste, pour un attaquant
controlant r noeuds malveillants, a monopoliser l'integralite de ces slots pour
isoler la victime du reseau honnete et lui presenter une vue falsifiee de la
blockchain.

Sur reseau mobile, les handovers et pertes de signal forcent regulierement le
wallet a se reconnecter. Notre question de recherche : **la degradation du signal
mobile augmente-t-elle la fenetre temporelle durant laquelle un attaquant peut
capturer les slots de connexion, par rapport a un reseau filaire stable ?**

Perimetre exclu (accord avec l'encadrant) : la formalisation analytique complete
du modele de Markov, et l'etude isolee de l'effet de la mobilite sans degradation
de signal.

## 2. Methodologie

### 2.1 Plateforme

Simulation realisee avec NS-3 (module core, network, internet, point-to-point),
sur WSL2 Ubuntu 22.04. Le code source complet est dans `src/`.

### 2.2 Modele du wallet SPV (`SpvWallet`)

Application NS-3 custom implementant une machine a etats a 5 etats
(S0 Connecte, S1 Mobilite, S2 Attente reconnexion, S3 Bootstrap, S4 Eclipse),
avec K=8 slots de connexion. Lors du remplissage des slots (initial ou apres
reconnexion), les noeuds attaquants sont prioritaires jusqu'a epuisement de r,
puis les slots restants sont completes par des noeuds honnetes -- ce qui
reflete l'avantage structurel de l'attaquant (annonces d'adresses plus rapides,
plus de connexions simultanees disponibles) documente dans la litterature sur
l'attaque originale.

### 2.3 Modele de signal mobile (`SignalModel`)

Processus stochastique generant des evenements a intervalles suivant une loi
exponentielle (parametre : intervalle moyen), chaque evenement produisant une
valeur de SINR suivant une loi normale (parametres : moyenne, ecart-type). Une
valeur sous le seuil configure (5 dB) declenche une transition vers l'etat S2 ;
un retour au-dessus du seuil declenche le retour vers S3 (bootstrap).

### 2.4 Scenarios compares

| Scenario | Intervalle moyen (s) | SINR moyen (dB) | Ecart-type | Interpretation |
|---|---|---|---|---|
| Filaire (reference) | - (desactive) | - | - | Pas de perturbation reseau |
| 4G mobilite moderee | 15 | 8 | 2 | Contexte pieton, peu d'evenements |
| 4G mobilite elevee | 5 | 6 | 3 | Contexte vehicule, handovers frequents |
| 4G degrade (rural) | 3 | 2 | 4 | Zone de faible couverture / brouillage |

Chaque scenario est teste avec r in {2, 4, 6, 8} noeuds attaquants, repete 5 fois
(graines aleatoires distinctes) pour lisser la variance stochastique du modele
de signal. Total : 80 simulations, duree simulee 60s chacune.

## 3. Resultats

### 3.1 Occupation adverse des slots (voir `results/occupation_adverse.png`)

Note : dans les graphiques `occupation_adverse.png` et `taux_eclipse.png`, les 4 courbes par scenario sont parfaitement superposees (une seule couleur visible) -- ce nest pas une erreur mais un resultat attendu : le mecanisme de monopolisation des slots ne depend que de r, pas du scenario reseau. Seule la duree dexposition (fig. fenetre_vulnerabilite.png) varie selon le scenario.

Le taux d'occupation adverse croit lineairement avec r, independamment du
scenario reseau -- resultat attendu, la logique de monopolisation des slots
etant identique dans tous les scenarios. A r=8=K, l'occupation atteint 100%
(eclipse totale garantie) dans tous les cas.

### 3.2 Fenetre de vulnerabilite S2+S3 (voir `results/fenetre_vulnerabilite.png`)

C'est le resultat central du projet. A r=4 (occupation partielle, cas non trivial) :

| Scenario | Temps S0 (s) | Temps S2 (s) | Temps S3 (s) |
|---|---|---|---|
| Filaire | 60 | 0 | 0 |
| 4G modere | 60 | 0 | 0 |
| 4G eleve | 34.3 | 23.7 | 2.0 |
| 4G degrade | 28.3 | 29.7 | 2.0 |

Le wallet passe jusqu'a **~53% du temps de simulation en etat vulnerable
(S2+S3)** dans le scenario degrade, contre 0% dans les scenarios filaire et
4G modere. Le scenario "4G modere" ne produit quasiment aucune degradation
mesurable : sous un certain seuil de frequence/severite des evenements
signal, le risque additionnel reste negligeable.

### 3.3 Taux d'eclipse (voir `results/taux_eclipse.png`)

Le nombre d'eclipses detectees suit directement le taux d'occupation adverse ;
la degradation du signal ne cree pas d'eclipse a elle seule (elle ne modifie
pas le nombre d'attaquants), mais **augmente la duree pendant laquelle le
wallet est expose** au risque de reconnexion vers des pairs malveillants.

## 4. Recommandations

1. **Diversifier les sources de decouverte de pairs** lors du bootstrap
   post-degradation (DNS seeds multiples, cache de pairs anciens ne
   dependant pas uniquement des annonces recues juste avant la coupure).
2. **Limiter le taux de remplacement des slots** apres une reconnexion :
   privilegier une reconnexion graduelle plutot qu'un remplissage
   immediat et complet, qui favorise l'attaquant le plus reactif.
3. **Augmenter K sur reseau mobile identifie comme instable** : le cout
   memoire d'un plus grand nombre de connexions sortantes reste marginal
   face au risque observe dans les scenarios degrades.
4. **Alerter l'utilisateur** lorsque le wallet reste anormalement longtemps
   en etat de reconnexion (S2/S3), signal possible d'un environnement
   reseau hostile ou d'une tentative d'eclipse en cours.

## 5. Limites

- Modele de signal stochastique simplifie (pas de module LTE physique complet)
- Pas de modelisation du protocole P2P complet (annonces de blocs, propagation)
- Formalisation analytique du modele de Markov hors perimetre (cf. accord
  avec l'encadrant)
- Effet de la mobilite geographique isole non etudie (hors perimetre)

## 6. Reproductibilite

Voir `README.md` a la racine du depot pour les instructions completes de
compilation et d'execution.
