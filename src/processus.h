#ifndef PROCESSUS_H
#define PROCESSUS_H

#define MAX_SEGMENTS_GANTT 250

typedef struct {
    int debut;
    int fin;
} Segment_Diagramme_Gantt;

typedef struct {
    char nom[20];
    int arrivee;
    int duree;
    int priorite;
    int restant;
    int temps_debut;    

    int temps_sortie;
    Segment_Diagramme_Gantt diagramme_gantt[MAX_SEGMENTS_GANTT];
    int nb_segments;
} Processus;

void ordonnancer(Processus tableau_processus[], int nombre_processus);
void afficher_resultats(Processus tableau_processus[], int nombre_processus);

#endif
