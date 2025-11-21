#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    printf("===== Ordonnancement FIFO =====\n");
    for (int i = 0; i < nombre_processus - 1; i++) {
        for (int j = i + 1; j < nombre_processus; j++) {
            if (tableau_processus[j].arrivee < tableau_processus[i].arrivee) {
                Processus processus_temporaire = tableau_processus[i];
                tableau_processus[i] = tableau_processus[j];
                tableau_processus[j] = processus_temporaire;
            }
        }
    }
    int temps = 0;
    for (int i = 0; i < nombre_processus; i++) {
        tableau_processus[i].nb_segments = 0;

        int temps_debut = (temps < tableau_processus[i].arrivee) ? tableau_processus[i].arrivee : temps;
        int temps_fin = temps_debut + tableau_processus[i].duree;

        if (temps_fin > temps_debut) {
            tableau_processus[i].diagramme_gantt[0].debut = temps_debut;
            tableau_processus[i].diagramme_gantt[0].fin = temps_fin;
            tableau_processus[i].nb_segments = 1;
        }

        temps = temps_fin;
        tableau_processus[i].temps_sortie = temps_fin;
    }
}
