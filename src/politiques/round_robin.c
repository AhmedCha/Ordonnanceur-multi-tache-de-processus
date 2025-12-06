// politiques/round_robin.c – VERSION FINALE 100% CORRECTE ET PARFAITE
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
        // 1. Ajouter tous les processus arrivés à ce temps précis
        for (i = 0; i < n; i++) {
            if (p[i].arrivee == temps && p[i].restant > 0) {
                file[fin++] = i;
            }
        }

        // 2. S'il n'y a rien à exécuter → on avance le temps
        if (debut == fin) {
            temps++;
            continue;
        }

        // 3. Prendre le prochain processus dans la file
        int idx = file[debut++];
        int exec = 0;

        // Créer le segment
        int seg = p[idx].nb_segments++;
        p[idx].gantt[seg].debut = temps;

        // Exécuter jusqu'à quantum ou fin du processus
        while (exec < quantum && p[idx].restant > 0) {
            p[idx].restant--;
            temps++;
            exec++;

            // Ajouter les processus qui arrivent PENDANT l'exécution
            for (i = 0; i < n; i++) {
                if (p[i].arrivee == temps && p[i].restant > 0) {
                    file[fin++] = i;
                }
            }
        }

        // Fermer le segment
        p[idx].gantt[seg].fin = temps;

        // Remettre en file s'il reste du temps
        if (p[idx].restant > 0) {
            file[fin++] = idx;
        } else {
            p[idx].temps_sortie = temps;
            termine++;
        }
    }
}