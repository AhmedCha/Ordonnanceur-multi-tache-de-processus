#include "../processus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_QUEUES 3
#define TIME_QUANTUM_1 4
#define TIME_QUANTUM_2 8
#define TIME_QUANTUM_3 16
#define AGING_INTERVAL 5

typedef struct {
    Processus** files[NUM_QUEUES];
    int debut[NUM_QUEUES];
    int fin[NUM_QUEUES];
    int taille[NUM_QUEUES];
    int capacite[NUM_QUEUES];
    int quantums[NUM_QUEUES];
} OrdonnanceurMLFQ;


OrdonnanceurMLFQ* mlfq_creer() {
    OrdonnanceurMLFQ* ordonnanceur = malloc(sizeof(OrdonnanceurMLFQ));
    
    for (int i = 0; i < NUM_QUEUES; i++) {
        ordonnanceur->capacite[i] = 50;
        ordonnanceur->files[i] = malloc(sizeof(Processus*) * ordonnanceur->capacite[i]);
        ordonnanceur->debut[i] = 0;
        ordonnanceur->fin[i] = 0;
        ordonnanceur->taille[i] = 0;
    }
    
    ordonnanceur->quantums[0] = TIME_QUANTUM_1;
    ordonnanceur->quantums[1] = TIME_QUANTUM_2;
    ordonnanceur->quantums[2] = TIME_QUANTUM_3;
    
    return ordonnanceur;
}

void mlfq_ajouter_processus(OrdonnanceurMLFQ* ordonnanceur, Processus* processus, int niveau_priorite) {
    if (niveau_priorite < 0) niveau_priorite = 0;
    if (niveau_priorite >= NUM_QUEUES) niveau_priorite = NUM_QUEUES - 1;
    
    int index_file = niveau_priorite;
    
    if (ordonnanceur->taille[index_file] >= ordonnanceur->capacite[index_file]) {
        return;
    }
    
    ordonnanceur->files[index_file][ordonnanceur->fin[index_file]] = processus;
    ordonnanceur->fin[index_file] = (ordonnanceur->fin[index_file] + 1) % ordonnanceur->capacite[index_file];
    ordonnanceur->taille[index_file]++;
}

Processus* mlfq_obtenir_prochain(OrdonnanceurMLFQ* ordonnanceur) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        if (ordonnanceur->taille[i] > 0) {
            Processus* prochain_processus = ordonnanceur->files[i][ordonnanceur->debut[i]];
            ordonnanceur->debut[i] = (ordonnanceur->debut[i] + 1) % ordonnanceur->capacite[i];
            ordonnanceur->taille[i]--;
            return prochain_processus;
        }
    }
    return NULL;
}

void mlfq_vieillissement(OrdonnanceurMLFQ* ordonnanceur, int temps_actuel) {
    if (temps_actuel % AGING_INTERVAL != 0) {
        return;
    }
    
    for (int index_file = NUM_QUEUES - 1; index_file > 0; index_file--) {
        if (ordonnanceur->taille[index_file] > 0) {
            Processus* processus_vieilli = ordonnanceur->files[index_file][ordonnanceur->debut[index_file]];
            
            ordonnanceur->debut[index_file] = (ordonnanceur->debut[index_file] + 1) % ordonnanceur->capacite[index_file];
            ordonnanceur->taille[index_file]--;
            
            int nouvelle_file = index_file - 1;
            ordonnanceur->files[nouvelle_file][ordonnanceur->fin[nouvelle_file]] = processus_vieilli;
            ordonnanceur->fin[nouvelle_file] = (ordonnanceur->fin[nouvelle_file] + 1) % ordonnanceur->capacite[nouvelle_file];
            ordonnanceur->taille[nouvelle_file]++;
            
            printf("Vieillissement: Processus %s promu de la file %d à %d au temps %d\n",
                   processus_vieilli->nom, index_file, nouvelle_file, temps_actuel);
            break;
        }
    }
}

void mlfq_nettoyer(OrdonnanceurMLFQ* ordonnanceur) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        free(ordonnanceur->files[i]);
    }
    free(ordonnanceur);
}

void ordonnancer(Processus processus[], int count) {
    OrdonnanceurMLFQ* mlfq = mlfq_creer();
    int temps_actuel = 0;
    int completes = 0;
    Processus* processus_courant = NULL;
    int quantum_restant = 0;
    
    printf("\n=== Ordonnancement MLFQ avec Vieillissement ===\n");
    
    for (int i = 0; i < count; i++) {
        processus[i].restant = processus[i].duree;
        processus[i].nb_segments = 0;
    }
    
    while (completes < count) {
        for (int i = 0; i < count; i++) {
            if (processus[i].arrivee == temps_actuel) {
                int file_initiale = processus[i].priorite % NUM_QUEUES;
                mlfq_ajouter_processus(mlfq, &processus[i], file_initiale);
                printf("Temps %d: %s arrivé -> file %d\n", temps_actuel, processus[i].nom, file_initiale);
            }
        }
        
        mlfq_vieillissement(mlfq, temps_actuel);
        
        if (processus_courant == NULL || quantum_restant == 0) {
            if (processus_courant != NULL && processus_courant->restant > 0) {
                int nouvelle_file = (processus_courant->priorite + 1) % NUM_QUEUES;
                mlfq_ajouter_processus(mlfq, processus_courant, nouvelle_file);
                printf("Temps %d: %s rétrogradé vers file %d\n", temps_actuel, processus_courant->nom, nouvelle_file);
            }
            
            processus_courant = mlfq_obtenir_prochain(mlfq);
            if (processus_courant != NULL) {
                quantum_restant = mlfq->quantums[processus_courant->priorite % NUM_QUEUES];
                printf("Temps %d: Exécution de %s (file %d, quantum: %d)\n", 
                       temps_actuel, processus_courant->nom, 
                       processus_courant->priorite % NUM_QUEUES, quantum_restant);
            }
        }
        
        if (processus_courant != NULL) {
            if (processus_courant->nb_segments == 0 || processus_courant->diagramme_gantt[processus_courant->nb_segments - 1].fin != temps_actuel) {
                processus_courant->diagramme_gantt[processus_courant->nb_segments].debut = temps_actuel;
                processus_courant->nb_segments++;
            }
            processus_courant->diagramme_gantt[processus_courant->nb_segments - 1].fin = temps_actuel + 1;

            processus_courant->restant--;
            quantum_restant--;
            
            if (processus_courant->restant == 0) {
                completes++;
                processus_courant->temps_sortie = temps_actuel + 1;
                printf("Temps %d: %s TERMINÉ\n", temps_actuel + 1, processus_courant->nom);
                processus_courant = NULL;
                quantum_restant = 0;
            }
        }
        
        if (processus_courant == NULL) {
            int processus_en_attente = 0;
            for (int i = 0; i < NUM_QUEUES; i++) {
                if (mlfq->taille[i] > 0) {
                    processus_en_attente = 1;
                    break;
                }
            }
            
            int processus_a_venir = 0;
            for (int i = 0; i < count; i++) {
                if (processus[i].arrivee > temps_actuel && processus[i].restant > 0) {
                    processus_a_venir = 1;
                    break;
                }
            }
            
            if (!processus_en_attente && !processus_a_venir) {
                break;
            }
        }
        
        temps_actuel++;
        
        if (temps_actuel > 1000) {
            printf("ERREUR: Timeout - boucle infinie possible\n");
            break;
        }
    }
    
    mlfq_nettoyer(mlfq);

    printf("=== Ordonnancement MLFQ Terminé ===\n");
}
