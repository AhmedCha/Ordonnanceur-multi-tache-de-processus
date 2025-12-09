#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    int temps = 0;
    int nb_termines = 0;
    int i;

    for (i = 0; i < nombre_processus; i++) {
        tableau_processus[i].restant = tableau_processus[i].duree;
        tableau_processus[i].nb_segments = 0;
        tableau_processus[i].temps_sortie = -1;
    }

    int processus_courant = -1;
    int debut_segment = 0;

    while (nb_termines < nombre_processus) {
        int meilleur = -1;
        int meilleure_priorite = -1;

        for (i = 0; i < nombre_processus; i++) {
            if (tableau_processus[i].arrivee <= temps && 
                tableau_processus[i].restant > 0 && 
                tableau_processus[i].priorite > meilleure_priorite) {
                
                meilleure_priorite = tableau_processus[i].priorite;
                meilleur = i;
            }
        }

        if (meilleur == -1) {
            temps++;
            continue;
        }

        if (processus_courant != -1 && processus_courant != meilleur) {
            Processus *p = &tableau_processus[processus_courant];
            int idx = p->nb_segments++;
            if (idx < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[idx].debut = debut_segment;
                p->diagramme_gantt[idx].fin = temps;
            }
        }

        if (processus_courant != meilleur) {
            processus_courant = meilleur;
            debut_segment = temps;
        }

        tableau_processus[processus_courant].restant--;
        temps++;

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