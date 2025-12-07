#ifndef PROCESSUS_H
#define PROCESSUS_H

#define MAX_SEGMENTS_GANTT 250

typedef struct {
    int debut;
    int fin;
} Segment_Diagramme_Gantt;

/* Structure Processus — VERSION FINALE CORRIGÉE */
typedef struct {
    char nom[20];
    int arrivee;
    int duree;
    int priorite;

    /* Champs utilisés par les algorithmes */
    int restant;               // temps restant à exécuter
    int temps_debut;           // moment du premier lancement (optionnel)
    int temps_sortie;          // moment de fin d'exécution

    /* Pour le diagramme de Gantt */
    Segment_Diagramme_Gantt diagramme_gantt[MAX_SEGMENTS_GANTT];
    int nb_segments;

    int temps_attente;         // temps total passé en attente (pour calculer aging)
    int priorite_dynamique;    // priorité qui évolue avec le temps (aging)
    int dernier_boost;         // optionnel : dernier moment où la priorité a été boostée
} Processus;

/* Prototypes des fonctions communes */
void afficher_resultats(Processus tableau_processus[], int nombre_processus);

#endif // PROCESSUS_H