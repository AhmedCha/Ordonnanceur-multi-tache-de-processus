// aging.so — VERSION CORRIGÉE ET PARFAITE (vrai aging + RR par priorité)
#include "../processus.h"
#include <stdio.h>
#include <stdlib.h>

static int quantum = 2;

void definir_quantum(int q) {
    if (q > 0) quantum = q;
}

#define PRIORITE_MAX 5
#define PRIORITE_MIN 1

void ordonnancer(Processus processus[], int nombre) {
    printf("=== MLFQ avec Aging réel (quantum=%d) ===\n", quantum);

    // Initialisation
    for (int i = 0; i < nombre; i++) {
        processus[i].restant = processus[i].duree;
        processus[i].nb_segments = 0;
        processus[i].temps_sortie = -1;
        processus[i].temps_attente = 0;  // On va utiliser ça pour l'aging
    }

    int temps = 0;
    int termines = 0;
    int courant = -1;
    int debut_quantum = 0;

    while (termines < nombre) {
        // 1. AGING : tous les processus en attente augmentent leur priorité de 1 à chaque tour
        for (int i = 0; i < nombre; i++) {
            if (processus[i].restant > 0 && 
                processus[i].arrivee <= temps && 
                i != courant && 
                processus[i].priorite < PRIORITE_MAX) {
                
                processus[i].priorite++;
                // printf("[AGING t=%d] %s → prio %d\n", temps, processus[i].nom, processus[i].priorite);
            }
        }

        // 2. Trouver le processus avec la priorité la plus haute
        int meilleur = -1;
        int meilleure_prio = -1;

        for (int i = 0; i < nombre; i++) {
            if (processus[i].restant > 0 && processus[i].arrivee <= temps) {
                if (processus[i].priorite > meilleure_prio) {
                    meilleure_prio = processus[i].priorite;
                    meilleur = i;
                }
            }
        }

        // Si aucun processus prêt → attente
        if (meilleur == -1) {
            temps++;
            continue;
        }

        // 3. Préemption si nécessaire
        if (courant != meilleur && courant != -1 && processus[courant].restant > 0) {
            // Sauvegarde du segment précédent
            Processus* p = &processus[courant];
            if (p->nb_segments < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[p->nb_segments].debut = debut_quantum;
                p->diagramme_gantt[p->nb_segments].fin = temps;
                p->nb_segments++;
            }
            printf("[t=%d] Préemption : %s → %s (prio %d > %d)\n",
                   temps, p->nom, processus[meilleur].nom,
                   processus[meilleur].priorite, p->priorite);
        }

        courant = meilleur;
        if (processus[courant].nb_segments == 0 || 
            processus[courant].diagramme_gantt[processus[courant].nb_segments-1].fin != temps) {
            debut_quantum = temps;
        }

        // Exécution d'une unité
        processus[courant].restant--;
        temps++;

        // Mise à jour du segment courant
        int seg = processus[courant].nb_segments;
        if (seg == 0 || processus[courant].diagramme_gantt[seg-1].fin != temps-1) {
            if (seg < MAX_SEGMENTS_GANTT) {
                processus[courant].diagramme_gantt[seg].debut = temps-1;
                processus[courant].diagramme_gantt[seg].fin = temps;
                processus[courant].nb_segments++;
            }
        } else {
            processus[courant].diagramme_gantt[seg-1].fin = temps;
        }

        // Fin du processus ?
        if (processus[courant].restant == 0) {
            processus[courant].temps_sortie = temps;
            printf("[t=%d] %s terminé !\n", temps, processus[courant].nom);
            termines++;
            courant = -1;
            continue;
        }

        // Quantum épuisé ?
        if ((temps - debut_quantum) >= quantum) {
            Processus* p = &processus[courant];
            if (p->nb_segments < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[p->nb_segments].debut = debut_quantum;
                p->diagramme_gantt[p->nb_segments].fin = temps;
                p->nb_segments++;
            }

            if (p->priorite > PRIORITE_MIN) {
                p->priorite--;
                printf("[t=%d] %s descend → prio %d (quantum épuisé)\n", temps, p->nom, p->priorite);
            }
            courant = -1;
        }
    }

    printf("=== Ordonnancement terminé à t=%d ===\n", temps);
}