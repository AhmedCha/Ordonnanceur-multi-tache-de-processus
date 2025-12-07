// politiques/aging.c — AGING + QUANTUM + PRÉEMPTION + RR (plus grand = plus prioritaire)
#include "../processus.h"
#include <stdio.h>

static int quantum = 4;  // Quantum par défaut (modifiable via l'interface)

void definir_quantum(int q) {
    if (q > 0) quantum = q;
}

void ordonnancer(Processus procs[], int n) {
    int temps = 0;
    int termines = 0;
    int i;

    // Initialisation
    for (i = 0; i < n; i++) {
        procs[i].restant = procs[i].duree;
        procs[i].nb_segments = 0;
        procs[i].temps_sortie = -1;
        procs[i].priorite_dynamique = procs[i].priorite;
    }

    int courant = -1;
    int debut_quantum = 0;
    int temps_dans_quantum = 0;

    while (termines < n) {
        // 1. AGING : chaque tick, les processus en attente gagnent +1 priorité
        for (i = 0; i < n; i++) {
            if (procs[i].restant > 0 && procs[i].arrivee <= temps && i != courant) {
                procs[i].priorite_dynamique++;
            }
        }

        // 2. Trouver le processus avec la PLUS HAUTE priorité
        int meilleur = -1;
        int meilleure_prio = -1;
        int candidats[100];
        int nb_candidats = 0;

        for (i = 0; i < n; i++) {
            if (procs[i].arrivee <= temps && procs[i].restant > 0) {
                if (procs[i].priorite_dynamique > meilleure_prio) {
                    meilleure_prio = procs[i].priorite_dynamique;
                    nb_candidats = 1;
                    candidats[0] = i;
                } else if (procs[i].priorite_dynamique == meilleure_prio) {
                    candidats[nb_candidats++] = i;
                }
            }
        }

        if (meilleur == -1) { temps++; continue; }

        // 3. Round-Robin parmi les candidats
        static int rr_index = 0;
        int selectionne = candidats[rr_index % nb_candidats];
        rr_index++;

        // 4. Préemption si nécessaire
        if (courant != -1 && courant != selectionne) {
            int idx = procs[courant].nb_segments++;
            if (idx < MAX_SEGMENTS_GANTT) {
                procs[courant].diagramme_gantt[idx].debut = debut_quantum;
                procs[courant].diagramme_gantt[idx].fin = temps;
            }
        }

        // 5. Changer de processus
        if (courant != selectionne) {
            courant = selectionne;
            debut_quantum = temps;
            temps_dans_quantum = 0;
        }

        // 6. Exécuter 1 unité
        procs[courant].restant--;
        temps_dans_quantum++;
        temps++;

        // 7. Fin du processus
        if (procs[courant].restant == 0) {
            int idx = procs[courant].nb_segments++;
            if (idx < MAX_SEGMENTS_GANTT) {
                procs[courant].diagramme_gantt[idx].debut = debut_quantum;
                procs[courant].diagramme_gantt[idx].fin = temps;
            }
            procs[courant].temps_sortie = temps;
            termines++;
            courant = -1;
            continue;
        }

        // 8. Quantum épuisé → baisse de priorité + libération
        if (temps_dans_quantum >= quantum) {
            if (procs[courant].priorite_dynamique > 1) {
                procs[courant].priorite_dynamique--;
            }
            courant = -1;
        }
    }
}