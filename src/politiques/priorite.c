// politiques/priorite.c — PRIORITÉ PRÉEMPTIVE (plus GRAND = plus prioritaire)
#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    int temps = 0;
    int nb_termines = 0;
    int i;

    // Initialisation
    for (i = 0; i < nombre_processus; i++) {
        tableau_processus[i].restant = tableau_processus[i].duree;
        tableau_processus[i].nb_segments = 0;
        tableau_processus[i].temps_sortie = -1;
    }

    int processus_courant = -1;     // processus en cours d'exécution
    int debut_segment = 0;          // début du segment courant

    while (nb_termines < nombre_processus) {
        // 1. Trouver le processus prêt avec la PLUS HAUTE priorité (plus GRAND nombre)
        int meilleur = -1;
        int meilleure_priorite = -1;  // très petite au départ

        for (i = 0; i < nombre_processus; i++) {
            if (tableau_processus[i].arrivee <= temps && 
                tableau_processus[i].restant > 0 && 
                tableau_processus[i].priorite > meilleure_priorite) {
                
                meilleure_priorite = tableau_processus[i].priorite;
                meilleur = i;
            }
        }

        // Aucun processus prêt → on avance le temps
        if (meilleur == -1) {
            temps++;
            continue;
        }

        // 2. PRÉEMPTION : si un processus plus prioritaire arrive, on coupe l'actuel
        if (processus_courant != -1 && processus_courant != meilleur) {
            // On termine le segment du processus interrompu
            Processus *p = &tableau_processus[processus_courant];
            int idx = p->nb_segments++;
            if (idx < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[idx].debut = debut_segment;
                p->diagramme_gantt[idx].fin = temps;
            }
        }

        // 3. Changer de processus (ou continuer)
        if (processus_courant != meilleur) {
            processus_courant = meilleur;
            debut_segment = temps;
        }

        // 4. Exécuter 1 unité de temps
        tableau_processus[processus_courant].restant--;
        temps++;

        // 5. Si terminé
        if (tableau_processus[processus_courant].restant == 0) {
            Processus *p = &tableau_processus[processus_courant];
            int idx = p->nb_segments++;
            if (idx < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[idx].debut = debut_segment;
                p->diagramme_gantt[idx].fin = temps;
            }
            p->temps_sortie = temps;
            nb_termines++;
            processus_courant = -1;
        }
    }
}