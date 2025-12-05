    #include <stdio.h>
    #include <string.h>
    #include "../processus.h"

    void ordonnancer(Processus p[], int n) {
        int temps = 0, i, j;

        // Tri par date d'arrivée
        for (i = 0; i < n-1; i++)
            for (j = i+1; j < n; j++)
                if (p[i].arrivee > p[j].arrivee) {
                    Processus temp = p[i];
                    p[i] = p[j];
                    p[j] = temp;
                }

        for (i = 0; i < n; i++) {
            if (temps < p[i].arrivee) temps = p[i].arrivee;

            int seg = p[i].nb_segments++;
            p[i].gantt[seg].debut = temps;
            p[i].gantt[seg].fin   = temps + p[i].duree;

            temps += p[i].duree;
            p[i].temps_sortie = temps;
        }
    }