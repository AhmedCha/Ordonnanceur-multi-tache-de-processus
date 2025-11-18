#include <stdio.h>
#include <stdlib.h>
#include "../processus.h"

void vider_tampon_entree() {
    int caractere;
    while ((caractere = getchar()) != '\n' && caractere != EOF) { }
}
void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    int quantum;
    printf("===== Ordonnancement Round Robin =====\n");
    printf("Entrez la valeur du quantum : ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        printf("Quantum invalide.\n");
        vider_tampon_entree();
        return;
    }
    vider_tampon_entree();

    int temps_restant[nombre_processus];
    for (int i = 0; i < nombre_processus; i++) temps_restant[i] = tableau_processus[i].duree;

    for (int i = 0; i < nombre_processus - 1; i++) {
        for (int j = i + 1; j < nombre_processus; j++) {
            if (tableau_processus[j].arrivee < tableau_processus[i].arrivee) {
                Processus processus_temporaire = tableau_processus[i];
                tableau_processus[i] = tableau_processus[j];
                tableau_processus[j] = processus_temporaire;
                int temps_restant_temporaire = temps_restant[i];
                temps_restant[i] = temps_restant[j];
                temps_restant[j] = temps_restant_temporaire;
            }
        }
    }

    int temps = 0;
    int nb_termines = 0;

    int file_attente[2 * nombre_processus];
    int debut = 0, fin = 0;

    for (int i = 0; i < nombre_processus; i++) {
        if (tableau_processus[i].arrivee <= 0 && temps_restant[i] > 0) file_attente[fin++] = i;
    }
    if (debut == fin) {
        temps = tableau_processus[0].arrivee;
        file_attente[fin++] = 0;
    }

    while (nb_termines < nombre_processus) {
        if (debut == fin) {
            temps++;
            for (int j = 0; j < nombre_processus; j++) {
                if (tableau_processus[j].arrivee == temps && temps_restant[j] > 0) file_attente[fin++] = j;
            }
            continue;
        }

        int i = file_attente[debut++];

        if (temps_restant[i] <= 0) continue;

        if (tableau_processus[i].arrivee > temps) temps = tableau_processus[i].arrivee;

        int duree_execution = (temps_restant[i] < quantum) ? temps_restant[i] : quantum;
        printf("%s s’exécute de %d à %d\n", tableau_processus[i].nom, temps, temps + duree_execution);

        int temps_debut_segment = temps;
        temps += duree_execution;
        temps_restant[i] -= duree_execution;

        for (int j = 0; j < nombre_processus; j++) {
            if (j == i) continue;
            if (tableau_processus[j].arrivee > temps_debut_segment && tableau_processus[j].arrivee <= temps && temps_restant[j] > 0) {
                int deja_dans_file = 0;
                for (int k = debut; k < fin; k++) {
                    if (file_attente[k] == j) { deja_dans_file = 1; break; }
                }
                if (!deja_dans_file) file_attente[fin++] = j;
            }
        }

        if (temps_restant[i] > 0) {
            file_attente[fin++] = i;
        } else {
            nb_termines++;
        }
    }
}
