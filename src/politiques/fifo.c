#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    printf("===== Ordonnancement FIFO =====\n");
    for (int i = 0; i < nombre_processus - 1; i++) {
        for (int j = i + 1; j < nombre_processus; j++) {
            if (tableau_processus[j].arrivee < tableau_processus[i].arrivee) {
                Processus processus_temporaire = tableau_processus[i];
                tableau_processus[i] = tableau_processus[j];
                tableau_processus[j] = processus_temporaire;
            }
        }
    }
    int temps = 0;
    for (int i = 0; i < nombre_processus; i++) {
        if (temps < tableau_processus[i].arrivee)
            temps = tableau_processus[i].arrivee;
        printf("%s s’exécute de %d à %d\n",
               tableau_processus[i].nom, temps, temps + tableau_processus[i].duree);
        temps += tableau_processus[i].duree;
    }
}
