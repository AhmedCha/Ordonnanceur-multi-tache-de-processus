#ifndef MLFQ_H
#define MLFQ_H

#include "../processus.h"

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

// Fonctions MLFQ
OrdonnanceurMLFQ* mlfq_creer();
void mlfq_ajouter_processus(OrdonnanceurMLFQ* ordonnanceur, Processus* processus, int niveau_priorite);
Processus* mlfq_obtenir_prochain(OrdonnanceurMLFQ* ordonnanceur);
void mlfq_vieillissement(OrdonnanceurMLFQ* ordonnanceur, int temps_actuel);
void mlfq_nettoyer(OrdonnanceurMLFQ* ordonnanceur);
void schedule_mlfq(Processus** processus, int count);

#endif
