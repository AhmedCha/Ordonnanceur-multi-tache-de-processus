#include <stdio.h>
#include "../processus.h"

#define PRIORITE_MAX     50
#define AGING_INCREMENT  1
#define AGING_INTERVAL   8

void ordonnancer(Processus T[], int n) {
    int temps = 0;
    int restant[n];
    int prio_dyn[n];

    for (int i = 0; i < n; i++) {
        restant[i] = T[i].duree;
        prio_dyn[i] = T[i].priorite;
        T[i].nb_segments = 0;
    }

    int fin = 0;
    int courant = -1;
    int debut_segment = 0;

    printf("===== Ordonnancement par Priorité Préemptive + Aging =====\n");

    while (fin < n) {
        if (temps > 0 && temps % AGING_INTERVAL == 0) {
            for (int i = 0; i < n; i++) {
                if (restant[i] > 0 && i != courant && T[i].arrivee <= temps) {
                    prio_dyn[i] += AGING_INCREMENT;
                    if (prio_dyn[i] > PRIORITE_MAX)
                        prio_dyn[i] = PRIORITE_MAX;
                }
            }
        }

        int meilleure_prio = -1;
        int selectionne = -1;
        for (int i = 0; i < n; i++) {
            if (restant[i] > 0 && T[i].arrivee <= temps && prio_dyn[i] > meilleure_prio) {
                meilleure_prio = prio_dyn[i];
                selectionne = i;
            }
        }

        if (selectionne == -1) {
            temps++;
            continue;
        }

        if (courant != -1 && courant != selectionne) {
            Processus* p = &T[courant];
            if (p->nb_segments < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[p->nb_segments].debut = debut_segment;
                p->diagramme_gantt[p->nb_segments].fin = temps;
                p->nb_segments++;
            }
        }

        if (courant != selectionne) {
            courant = selectionne;
            debut_segment = temps;
        }

        restant[courant]--;
        temps++;

        if (restant[courant] == 0) {
            Processus* p = &T[courant];
            if (p->nb_segments < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[p->nb_segments].debut = debut_segment;
                p->diagramme_gantt[p->nb_segments].fin = temps;
                p->nb_segments++;
            }
            p->temps_sortie = temps;
            fin++;
            courant = -1;
        }
    }
}
