#ifndef PROCESSUS_H
#define PROCESSUS_H

#define MAX_SEGMENTS_GANTT 100

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
    int temps_sortie;
    int nb_segments;
    Segment_Diagramme_Gantt gantt[MAX_SEGMENTS_GANTT];
} Processus;

#endif
