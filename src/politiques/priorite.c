#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus p[], int n) {
    int temps = 0, i, idx, min_prio;

    while (temps < 10000) {
        idx = -1; min_prio = 9999;
        for (i = 0; i < n; i++) {
            if (p[i].arrivee <= temps && p[i].restant > 0 && p[i].priorite < min_prio) {
                min_prio = p[i].priorite;
                idx = i;
            }
        }
        if (idx == -1) { temps++; continue; }

        int seg = p[idx].nb_segments++;
        p[idx].gantt[seg].debut = temps;
        p[idx].restant--;
        temps++;
        p[idx].gantt[seg].fin = temps;

        if (p[idx].restant == 0)
            p[idx].temps_sortie = temps;
    }
}