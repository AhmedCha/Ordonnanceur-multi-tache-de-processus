#ifndef PROCESS_H
#define PROCESS_H

typedef struct {
    char nom[20];
    int arrivee;
    int duree;
    int priorite;
    int restant;
} Processus;

void ordonnancer(Processus tableau_processus[], int nombre_processus);

#endif
