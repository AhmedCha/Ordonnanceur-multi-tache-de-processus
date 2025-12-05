#include <stdio.h>
#include "processus.h"

void ordonnancer(Processus p[], int n) {
    int temps = 0, i, idx, min_effective;

    while (temps < 10000) {
        // Vieillissement : +1 toutes les 10 unités
        if (temps % 10 == 0)
            for (i = 0; i < n; i++)
                if (p[i].restant > 0 && p[i].arrivee <= temps)
                    if (p[i].priorite > 1) p[i].priorite--;

        idx = -1; min_effective = 9999;
        for (i = 0; i < n; i++) {
            if (p[i].arrivee <= temps && p[i].restant > 0) {
                int prio = p[i].priorite;
                if (prio < min_effective) {
                    min_effective = prio;
                    idx = i;
                }
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
        else
            p[idx].priorite = 5;  // reset après exécution
    }
}