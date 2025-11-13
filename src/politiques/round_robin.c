// politiques/round_robin.c
#include <stdio.h>
#include <stdlib.h>
#include "../processus.h"

void ordonnancer(Processus tab[], int n) {
    int quantum;
    printf("===== Ordonnancement Round Robin =====\n");
    printf("Entrez la valeur du quantum : ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        printf("Quantum invalide.\n");
        return;
    }

    // copie des temps restants
    int reste[n];
    for (int i = 0; i < n; i++) reste[i] = tab[i].duree;

    // tri initial par date d'arrivée (stable)
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (tab[j].arrivee < tab[i].arrivee) {
                Processus tmp = tab[i];
                tab[i] = tab[j];
                tab[j] = tmp;
                int rtmp = reste[i];
                reste[i] = reste[j];
                reste[j] = rtmp;
            }
        }
    }

    int temps = 0;
    int termines = 0;

    // file simple (capacité suffisante : 2*n)
    int file[2 * n];
    int debut = 0, fin = 0;

    // ajouter tous les processus arrivés à t = 0
    for (int i = 0; i < n; i++) {
        if (tab[i].arrivee <= 0 && reste[i] > 0) file[fin++] = i;
    }
    // si aucun, avancer jusqu'au premier arrivé et l'ajouter
    if (debut == fin) {
        temps = tab[0].arrivee;
        file[fin++] = 0;
    }

    while (termines < n) {
        if (debut == fin) { // file vide => avancer le temps et ajouter arrivants
            temps++;
            for (int j = 0; j < n; j++) {
                if (tab[j].arrivee == temps && reste[j] > 0) file[fin++] = j;
            }
            continue;
        }

        int i = file[debut++]; // dépiler

        // si ce processus est déjà terminé (doublon ancien) -> ignorer
        if (reste[i] <= 0) continue;

        // si le processus n'est pas encore arrivé (rare si on gère les arrivées) -> avancer le temps
        if (tab[i].arrivee > temps) temps = tab[i].arrivee;

        int exec = (reste[i] < quantum) ? reste[i] : quantum;
        printf("%s s’exécute de %d à %d\n", tab[i].nom, temps, temps + exec);

        int t_debut = temps;
        temps += exec;
        reste[i] -= exec;

        // ajouter tous les processus arrivés pendant cette exécution (exclure i)
        for (int j = 0; j < n; j++) {
            if (j == i) continue; // ne pas ré-ajouter le processus en cours
            if (tab[j].arrivee > t_debut && tab[j].arrivee <= temps && reste[j] > 0) {
                // vérifier qu'il n'est pas déjà dans la file (parcourir file entre debut..fin-1)
                int deja = 0;
                for (int k = debut; k < fin; k++) {
                    if (file[k] == j) { deja = 1; break; }
                }
                if (!deja) file[fin++] = j;
            }
        }

        // si le processus n'est pas fini, le remettre en queue
        if (reste[i] > 0) {
            file[fin++] = i;
        } else {
            termines++;
        }
    }
}
