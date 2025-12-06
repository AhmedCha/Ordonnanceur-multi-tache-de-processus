#include "../processus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int quantum_aging = 2;

void definir_quantum(int q) {
    if (q > 0) {
        quantum_aging = q;
    }
}

#define SEUIL_AGING 8
#define PRIORITE_MAX 5
#define PRIORITE_MIN 1

typedef struct {
    int temps_attente;
    int dernier_reveil;
} AgingData;

AgingData donnees_aging[100];

void initialiser_aging(Processus processus[], int nombre) {
    for (int i = 0; i < nombre; i++) {
        donnees_aging[i].temps_attente = 0;
        donnees_aging[i].dernier_reveil = processus[i].arrivee;
    }
}

void appliquer_aging(Processus processus[], int nombre, int temps_courant) {
    for (int i = 0; i < nombre; i++) {
        if (processus[i].restant > 0 && 
            processus[i].arrivee <= temps_courant) {

            int attente = temps_courant - donnees_aging[i].dernier_reveil;
            
            if (attente >= SEUIL_AGING && processus[i].priorite < PRIORITE_MAX) {
                processus[i].priorite++;
                printf("[AGING t=%d] %s boosté → prio %d (attente: %d)\n",
                       temps_courant, processus[i].nom, 
                       processus[i].priorite, attente);
                
                donnees_aging[i].dernier_reveil = temps_courant;
            }
        }
    }

    printf("\nAppuyez sur une touche pour revenir au menu...\n");
}

void ordonnancer(Processus processus[], int nombre) {
    printf("===== MLFQ avec Aging (Quantum=%d, Seuil Aging=%d) =====\n",
           quantum_aging, SEUIL_AGING);
    
    for (int i = 0; i < nombre; i++) {
        processus[i].restant = processus[i].duree;
        processus[i].nb_segments = 0;
        processus[i].temps_sortie = -1;
    }
    initialiser_aging(processus, nombre);

    int temps = 0;
    int processus_termines = 0;
    int processus_courant = -1;
    int temps_debut_quantum = 0;
    int temps_execute_courant = 0;

    int prochain_index[PRIORITE_MAX + 1];
    for (int i = 0; i <= PRIORITE_MAX; i++) {
        prochain_index[i] = 0;
    }

    while (processus_termines < nombre) {
        while (processus_termines < nombre) {
    printf("\n=== BOUCLE t=%d ===\n", temps);
    printf("[ETAT GLOBAL] processus_courant=%d, execute=%d, termines=%d/%d\n",
           processus_courant, temps_execute_courant, processus_termines, nombre);
    
    if (processus_courant != -1) {
        printf("[PROCESSUS COURANT] %s: prio=%d, restant=%d\n",
               processus[processus_courant].nom,
               processus[processus_courant].priorite,
               processus[processus_courant].restant);
    }
    
    printf("[TOUS LES PROCESSUS]:\n");
    for (int i = 0; i < nombre; i++) {
        if (processus[i].restant > 0) {
            printf("  %s: arrivee=%d, prio=%d, restant=%d, arrivee<=temps=%d\n",
                   processus[i].nom, processus[i].arrivee,
                   processus[i].priorite, processus[i].restant,
                   (processus[i].arrivee <= temps));
        }
    }

        appliquer_aging(processus, nombre, temps);

        for (int i = 0; i < nombre; i++) {
            if (processus[i].arrivee == temps) {
                printf("[t=%d] %s arrivé (prio=%d, durée=%d)\n",
                       temps, processus[i].nom, 
                       processus[i].priorite, processus[i].duree);
            }
        }

int candidat = -1;
int priorite_max = 0;

if (processus_courant != -1 && temps_execute_courant >= quantum_aging) {
    printf("[QUANTUM] %s libéré après %d unités (prio=%d)\n", 
           processus[processus_courant].nom, temps_execute_courant, 
           processus[processus_courant].priorite);
    processus_courant = -1;
}

for (int i = 0; i < nombre; i++) {
    if (processus[i].restant > 0 && 
        processus[i].arrivee <= temps &&
        processus[i].priorite > priorite_max) {
        priorite_max = processus[i].priorite;
        candidat = i;
    }
}

if (candidat != -1) {
    int nb_egaux = 0;
    int indices_egaux[100];
    
    for (int i = 0; i < nombre; i++) {
        if (processus[i].restant > 0 && 
            processus[i].arrivee <= temps &&
            processus[i].priorite == priorite_max) {
            indices_egaux[nb_egaux++] = i;
        }
    }
    
    if (nb_egaux > 1) {
        candidat = indices_egaux[prochain_index[priorite_max] % nb_egaux];
        prochain_index[priorite_max] = (prochain_index[priorite_max] + 1) % nb_egaux;
        printf("[ROUND ROBIN] Sélection de %s parmi %d processus à prio %d\n",
               processus[candidat].nom, nb_egaux, priorite_max);
    }
}

if (candidat == -1) {
    temps++;
    continue;
}

        if (processus_courant != candidat) {
            if (processus_courant != -1 && processus[processus_courant].restant > 0) {
                Processus* p_prev = &processus[processus_courant];
                if (p_prev->nb_segments < MAX_SEGMENTS_GANTT) {
                    p_prev->diagramme_gantt[p_prev->nb_segments].debut = temps_debut_quantum;
                    p_prev->diagramme_gantt[p_prev->nb_segments].fin = temps;
                    p_prev->nb_segments++;
                }
                donnees_aging[processus_courant].dernier_reveil = temps;
                
                if (processus[candidat].priorite > processus[processus_courant].priorite) {
                    printf("[PRÉEMPTION t=%d] %s → %s (prio %d > %d)\n",
                           temps, processus[processus_courant].nom, processus[candidat].nom,
                           processus[candidat].priorite, processus[processus_courant].priorite);
                }
            }

            processus_courant = candidat;
            temps_debut_quantum = temps;
            temps_execute_courant = 0;
            
            printf("[t=%d] Exécution: %s (prio=%d, restant=%d)\n",
                   temps, processus[processus_courant].nom,
                   processus[processus_courant].priorite,
                   processus[processus_courant].restant);
        }
processus[processus_courant].restant--;
temps_execute_courant++;
temps++;

if (temps_execute_courant >= quantum_aging && processus[processus_courant].priorite > PRIORITE_MIN) {
    processus[processus_courant].priorite--;
    printf("[DÉGRADATION t=%d] %s → prio %d (quantum atteint)\n", 
           temps, processus[processus_courant].nom, processus[processus_courant].priorite);
}

int nb_seg = processus[processus_courant].nb_segments;
if (nb_seg == 0 ||
    processus[processus_courant].diagramme_gantt[nb_seg - 1].fin != temps - 1) {

    if (nb_seg < MAX_SEGMENTS_GANTT) {
        processus[processus_courant].diagramme_gantt[nb_seg].debut = temps - 1;
        processus[processus_courant].diagramme_gantt[nb_seg].fin = temps;
        processus[processus_courant].nb_segments++;
    }
} else {
    processus[processus_courant].diagramme_gantt[nb_seg - 1].fin = temps;
}

        int should_continue = 0;

        if (processus[processus_courant].restant == 0) {
            processus[processus_courant].temps_sortie = temps;
            Processus* p_term = &processus[processus_courant];
            if (p_term->nb_segments < MAX_SEGMENTS_GANTT) {
                p_term->diagramme_gantt[p_term->nb_segments].debut = temps_debut_quantum;
                p_term->diagramme_gantt[p_term->nb_segments].fin = temps;
                p_term->nb_segments++;
            }
            processus_termines++;
            printf("[t=%d] ✅ %s TERMINÉ\n", temps, processus[processus_courant].nom);
            processus_courant = -1;
            
        } else if (temps_execute_courant >= quantum_aging) {
            Processus* p_exp = &processus[processus_courant];
            if (p_exp->nb_segments < MAX_SEGMENTS_GANTT) {
                p_exp->diagramme_gantt[p_exp->nb_segments].debut = temps_debut_quantum;
                p_exp->diagramme_gantt[p_exp->nb_segments].fin = temps;
                p_exp->nb_segments++;
            }
            printf("[QUANTUM ATTEINT t=%d] %s a exécuté %d unités\n",
                   temps, processus[processus_courant].nom, temps_execute_courant);
            
            Processus* p = &processus[processus_courant];
            if (p->priorite > PRIORITE_MIN) {
                p->priorite--;
                printf("[DÉGRADATION] %s → prio %d\n", p->nom, p->priorite);
            }
            
            donnees_aging[processus_courant].dernier_reveil = temps;
            processus_courant = -1;
            should_continue = 1;
        }

        if (should_continue) {
            continue;
        }
    }

    printf("===== Ordonnancement terminé à t=%d =====\n", temps);
}}