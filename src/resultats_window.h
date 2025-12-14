#ifndef RESULTATS_WINDOW_H
#define RESULTATS_WINDOW_H

#include "processus.h"

/**
 * Affiche une fenêtre avec les résultats de l'ordonnancement
 * @param processus_list Tableau des processus ordonnancés
 * @param num_processus Nombre de processus dans le tableau
 */
void afficher_fenetre_resultats(Processus processus_list[], int num_processus);

#endif // RESULTATS_WINDOW_H