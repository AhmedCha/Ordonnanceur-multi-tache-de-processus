// politiques/round_robin.c – VERSION PRO (quantum passé en paramètre)
#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus p[], int n, int quantum) {
    if (quantum <= 0) quantum = 4;

    int temps = 0;
    int i;

    // Initialisation
    for (i = 0; i < n; i++) {
        p[i].restant = p[i].duree;
        p[i].nb_segments = 0;
        p[i].temps_sortie = -1;
    }

    int file[1000];
    int debut = 0, fin = 0;
    int termine = 0;

    while (termine < n) {
        // Ajouter les processus arrivés
        for (i = 0; i < n; i++) {
            if (p[i].arrivee == temps && p[i].restant > 0) {
                file[fin++] = i;
            }
        }

        if (debut == fin) {
            temps++;
            continue;
        }

        int idx = file[debut++];
        int exec = 0;

        int seg = p[idx].nb_segments++;
        p[idx].gantt[seg].debut = temps;

        while (exec < quantum && p[idx].restant > 0) {
            p[idx].restant--;
            temps++;
            exec++;

            for (i = 0; i < n; i++) {
                if (p[i].arrivee == temps && p[i].restant > 0) {
                    file[fin++] = i;
                }
            }
        }

        p[idx].gantt[seg].fin = temps;

        if (p[idx].restant > 0) {
            file[fin++] = idx;
        } else {
            p[idx].temps_sortie = temps;
            termine++;
        }
    }
}