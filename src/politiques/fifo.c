#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus tableau_processus[], int nombre_processus) {
    printf("===== Ordonnancement FIFO =====\n");
    
    for (int i = 0; i < nombre_processus - 1; i++) {
        int min_index = i;
        for (int j = i + 1; j < nombre_processus; j++) {
            if (tableau_processus[j].arrivee < tableau_processus[min_index].arrivee) {
                min_index = j;
            }
        }
        if (min_index != i) {
            Processus temp = tableau_processus[i];
            tableau_processus[i] = tableau_processus[min_index];
            tableau_processus[min_index] = temp;
        }
    }
    
    int temps_courant = 0;
    
    for (int i = 0; i < nombre_processus; i++) {
        tableau_processus[i].nb_segments = 0;
        tableau_processus[i].restant = tableau_processus[i].duree;
        
        if (temps_courant < tableau_processus[i].arrivee) {
            temps_courant = tableau_processus[i].arrivee;
        }
        
        tableau_processus[i].diagramme_gantt[0].debut = temps_courant;
        tableau_processus[i].diagramme_gantt[0].fin = temps_courant + tableau_processus[i].duree;
        tableau_processus[i].nb_segments = 1;
        
        temps_courant += tableau_processus[i].duree;
        tableau_processus[i].temps_sortie = temps_courant;
        
        printf("Processus %s exécuté de %d à %d\n", 
               tableau_processus[i].nom,
               tableau_processus[i].diagramme_gantt[0].debut,
               tableau_processus[i].diagramme_gantt[0].fin);
    }
}