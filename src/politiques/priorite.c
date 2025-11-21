#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    printf("===== Ordonnancement par Priorité Préemptive =====\n");
    int temps = 0;
    int temps_restant[nombre_processus];
    int nb_termines = 0;
    int i;

    for (i = 0; i < nombre_processus; i++) {
        temps_restant[i] = tableau_processus[i].duree;
        tableau_processus[i].nb_segments = 0;
    }

    int processus_courant = -1;
    int debut_execution = 0;

    while (nb_termines < nombre_processus) {
        int index_selectionne = -1;
        int max_priorite = -1;

        for (i = 0; i < nombre_processus; i++) {
            if (tableau_processus[i].arrivee <= temps && temps_restant[i] > 0) {
                if (tableau_processus[i].priorite > max_priorite) {
                    max_priorite = tableau_processus[i].priorite;
                    index_selectionne = i;
                }
            }
        }

        if (index_selectionne == -1) {
            temps++;
            continue;
        }

        if (processus_courant != index_selectionne) {
            if (processus_courant != -1) {
                Processus* processus_precedent = &tableau_processus[processus_courant];
                int index_segment = processus_precedent->nb_segments;
                
                if (index_segment < MAX_SEGMENTS_GANTT) {
                    processus_precedent->diagramme_gantt[index_segment].debut = debut_execution;
                    processus_precedent->diagramme_gantt[index_segment].fin = temps;
                    processus_precedent->nb_segments++;
                }
            }
            processus_courant = index_selectionne;
            debut_execution = temps;
        }

        temps_restant[processus_courant]--;
        temps++;

        if (temps_restant[processus_courant] == 0) {
            Processus* processus_termine = &tableau_processus[processus_courant];
            int index_segment = processus_termine->nb_segments;

            if (index_segment < MAX_SEGMENTS_GANTT) {
                processus_termine->diagramme_gantt[index_segment].debut = debut_execution;
                processus_termine->diagramme_gantt[index_segment].fin = temps;
                processus_termine->nb_segments++;
            }
            
            processus_termine->temps_sortie = temps;
            nb_termines++;
            processus_courant = -1;
            debut_execution = 0;
        }
    }
}
