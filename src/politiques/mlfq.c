#include <stdio.h>
#include "processus.h"

void ordonnancer(Processus p[], int n) {
    int temps = 0, i, idx;
    int file1[100], f1_debut=0, f1_fin=0;  // Q1: quantum 2
    int file2[100], f2_debut=0, f2_fin=0;  // Q2: quantum 4
    int file3[100], f3_debut=0, f3_fin=0;  // Q3: FCFS

    while (temps < 10000) {
        // Ajouter les nouveaux processus dans Q1
        for (i = 0; i < n; i++)
            if (p[i].arrivee == temps && p[i].restant > 0)
                file1[f1_fin++] = i;

        idx = -1;
        int q = 0;
        if (f1_debut < f1_fin) { idx = file1[f1_debut++]; q = 2; }
        else if (f2_debut < f2_fin) { idx = file2[f2_debut++]; q = 4; }
        else if (f3_debut < f3_fin) { idx = file3[f3_debut++]; q = 999; }

        if (idx == -1) { temps++; continue; }

        int seg = p[idx].nb_segments++;
        p[idx].gantt[seg].debut = temps;
        int exec = 0;
        while (exec < q && p[idx].restant > 0) {
            p[idx].restant--;
            temps++;
            exec++;

            for (i = 0; i < n; i++)
                if (p[i].arrivee == temps && p[i].restant > 0)
                    file1[f1_fin++] = i;
        }
        p[idx].gantt[seg].fin = temps;

        if (p[idx].restant > 0) {
            if (q == 2) file2[f2_fin++] = idx;
            else if (q == 4) file3[f3_fin++] = idx;
        } else {
            p[idx].temps_sortie = temps;
        }
    }
}