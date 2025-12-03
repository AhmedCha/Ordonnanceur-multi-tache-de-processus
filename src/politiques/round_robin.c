#include <stdio.h>
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
    for (int i = 0; i < nombre_processus; i++) {
        temps_restant[i] = tableau_processus[i].duree;
        tableau_processus[i].nb_segments = 0;
    }

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

#define TAILLE_FILE 1000
    int file_attente[TAILLE_FILE];
    int debut = 0, fin = 0;

    for (int i = 0; i < nombre_processus; i++) {
        if (tableau_processus[i].arrivee <= 0 && temps_restant[i] > 0) {
            file_attente[fin] = i;
            fin = (fin + 1) % TAILLE_FILE;
        }
    }
    if (debut == fin) {
        temps = tableau_processus[0].arrivee;
        file_attente[fin] = 0;
        fin = (fin + 1) % TAILLE_FILE;
    }

    while (nb_termines < nombre_processus) {
        if (debut == fin) {
            temps++;
            for (int j = 0; j < nombre_processus; j++) {
                if (tableau_processus[j].arrivee == temps && temps_restant[j] > 0) {
                    file_attente[fin] = j;
                    fin = (fin + 1) % TAILLE_FILE;
                }
            }
            continue;
        }

        int i = file_attente[debut];
        debut = (debut + 1) % TAILLE_FILE;

        if (temps_restant[i] <= 0) continue;

        if (tableau_processus[i].arrivee > temps) temps = tableau_processus[i].arrivee;

        int duree_execution = (temps_restant[i] < quantum) ? temps_restant[i] : quantum;
        
        int temps_debut_segment = temps;
        temps += duree_execution;
        temps_restant[i] -= duree_execution;

        Processus* p = &tableau_processus[i];
        if (p->nb_segments < MAX_SEGMENTS_GANTT) {
            p->diagramme_gantt[p->nb_segments].debut = temps_debut_segment;
            p->diagramme_gantt[p->nb_segments].fin = temps;
            p->nb_segments++;
        }

        for (int j = 0; j < nombre_processus; j++) {
            if (j == i) continue;
            if (tableau_processus[j].arrivee > temps_debut_segment && tableau_processus[j].arrivee <= temps && temps_restant[j] > 0) {
                int deja_dans_file = 0;
                int curseur = debut;
                while (curseur != fin) {
                    if (file_attente[curseur] == j) {
                        deja_dans_file = 1;
                        break;
                    }
                    curseur = (curseur + 1) % TAILLE_FILE;
                }

                if (!deja_dans_file) {
                    file_attente[fin] = j;
                    fin = (fin + 1) % TAILLE_FILE;
                }
            }
        }

        if (temps_restant[i] > 0) {
            file_attente[fin] = i;
            fin = (fin + 1) % TAILLE_FILE;
        } else {
            p->temps_sortie = temps;
            nb_termines++;
        }
    }
}
