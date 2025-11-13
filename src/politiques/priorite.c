#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tab[], int n) {
    printf("===== Ordonnancement par Priorité Préemptive =====\n");

    int temps = 0;
    int restant[n];
    int termine = 0;
    int i;

    for (i = 0; i < n; i++)
        restant[i] = tab[i].duree;

    int courant = -1;  // index du processus en cours
    int debut_segment = 0;

    while (termine < n) {
        int idx = -1;
        int max_priorite = -1;

        // Chercher le processus prêt avec la priorité la plus élevée
        for (i = 0; i < n; i++) {
            if (tab[i].arrivee <= temps && restant[i] > 0) {
                if (tab[i].priorite > max_priorite) {
                    max_priorite = tab[i].priorite;
                    idx = i;
                }
            }
        }

        if (idx == -1) {  // aucun processus prêt
            temps++;
            continue;
        }

        // Préemption : si le processus change, afficher le segment précédent
        if (courant != idx) {
            if (courant != -1) {
                printf("%s (priorité %d) s’exécute de %d à %d\n",
                       tab[courant].nom, tab[courant].priorite,
                       debut_segment, temps);
            }
            courant = idx;
            debut_segment = temps;
        }

        // Exécuter 1 unité
        restant[courant]--;
        temps++;

        if (restant[courant] == 0) {  // Processus terminé
            printf("%s (priorité %d) s’exécute de %d à %d\n",
                   tab[courant].nom, tab[courant].priorite,
                   debut_segment, temps);
            termine++;
            courant = -1;
        }
    }
}
