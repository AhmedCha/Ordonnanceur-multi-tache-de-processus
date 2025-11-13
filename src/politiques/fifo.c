#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tab[], int n) {
    printf("===== Ordonnancement FIFO =====\n");

    // tri par arrivée
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (tab[j].arrivee < tab[i].arrivee) {
                Processus tmp = tab[i];
                tab[i] = tab[j];
                tab[j] = tmp;
            }
        }
    }

    int temps = 0;
    for (int i = 0; i < n; i++) {
        if (temps < tab[i].arrivee)
            temps = tab[i].arrivee;
        printf("%s s’exécute de %d à %d\n",
               tab[i].nom, temps, temps + tab[i].duree);
        temps += tab[i].duree;
    }
}
