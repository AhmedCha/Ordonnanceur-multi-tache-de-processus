// processus.h
#ifndef PROCESSUS_H
#define PROCESSUS_H

typedef struct {
    char nom[20];
    int arrivee;
    int duree;
    int priorite;
    int restant;  // utile pour round robin
} Processus;

// Prototype commun pour toutes les politiques
void ordonnancer(Processus tab[], int n);

#endif
