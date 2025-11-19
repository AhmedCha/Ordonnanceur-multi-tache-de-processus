#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./processus.h"

const char* couleurs_processus[] = {
    "\033[41m",
    "\033[42m",
    "\033[43m",
    "\033[44m",
    "\033[45m",
    "\033[46m",
    "\033[47m",
};
#define COULEUR_RESET "\033[0m"
#define NOMBRE_COULEURS 7

int obtenir_indice_couleur(const char *nom) {
    unsigned int hachage = 0;
    for (int i = 0; nom[i] != '\0'; i++) {
        hachage = hachage * 31 + nom[i];
    }
    return hachage % NOMBRE_COULEURS;
}

void afficher_diagramme_gantt(Processus tableau_processus[], int nombre_processus) {
    if (nombre_processus == 0) return;

    int temps_maximum = 0;
    for (int i = 0; i < nombre_processus; i++) {
        if (tableau_processus[i].temps_sortie > temps_maximum) {
            temps_maximum = tableau_processus[i].temps_sortie;
        }
    }

    if (temps_maximum == 0) {
        printf("Aucune exécution enregistrée pour le Diagramme de Gantt.\n");
        return;
    }

    printf("\n===== Diagramme de Gantt (Unité de Temps: 1) =====\n");
    
    int transitions[2 * 100 * MAX_SEGMENTS_GANTT + 2];
    int nb_transitions = 0;
    transitions[nb_transitions++] = 0;

    for (int i = 0; i < nombre_processus; i++) {
        for (int s = 0; s < tableau_processus[i].nb_segments; s++) {
            transitions[nb_transitions++] = tableau_processus[i].diagramme_gantt[s].debut;
            transitions[nb_transitions++] = tableau_processus[i].diagramme_gantt[s].fin;
        }
    }
    
    for (int i = 0; i < nb_transitions; i++) {
        for (int j = i + 1; j < nb_transitions; j++) {
            if (transitions[i] > transitions[j]) {
                int temp = transitions[i];
                transitions[i] = transitions[j];
                transitions[j] = temp;
            }
        }
    }

    int transitions_uniques[2 * 100 * MAX_SEGMENTS_GANTT + 2];
    int nb_unique = 0;
    if (nb_transitions > 0) {
        transitions_uniques[nb_unique++] = transitions[0];
        for (int i = 1; i < nb_transitions; i++) {
            if (transitions[i] > transitions[i-1]) {
                transitions_uniques[nb_unique++] = transitions[i];
            }
        }
    }
    
    int largeur_terminal = 120;
    int ligne_actuelle_largeur = 0;

    for (int i = 0; i < nb_unique - 1; ) {
        ligne_actuelle_largeur = 0;
        int segment_initial = i;
        while (i < nb_unique - 1) {
            int debut_segment = transitions_uniques[i];
            int fin_segment = transitions_uniques[i+1];
            int duree_segment = fin_segment - debut_segment;
            
            int largeur_restante = largeur_terminal - ligne_actuelle_largeur;
            int duree_a_afficher = (largeur_restante - 1) / 3;

            if (duree_segment > duree_a_afficher) {
                if(duree_a_afficher <= 0){
                    break;
                }
                int new_fin = debut_segment + duree_a_afficher;

                for (int j = nb_unique; j > i + 1; j--) {
                    transitions_uniques[j] = transitions_uniques[j - 1];
                }
                transitions_uniques[i + 1] = new_fin;
                nb_unique++;
                
                fin_segment = new_fin;
                duree_segment = fin_segment - debut_segment;
            }
            
            int largeur_visuelle = duree_segment * 3 + 1;
            ligne_actuelle_largeur += largeur_visuelle;

            int index_processus_courant = -1;
            for (int p = 0; p < nombre_processus; p++) {
                for (int s = 0; s < tableau_processus[p].nb_segments; s++) {
                    Segment_Diagramme_Gantt segment = tableau_processus[p].diagramme_gantt[s];
                    if (debut_segment >= segment.debut && debut_segment < segment.fin) {
                        index_processus_courant = p;
                        break;
                    }
                }
                if (index_processus_courant != -1) break;
            }

            printf("|");
            if (index_processus_courant != -1) {
                Processus p = tableau_processus[index_processus_courant];
                int couleur_idx = obtenir_indice_couleur(p.nom);
                const char* nom_a_afficher = p.nom;
                int longueur_nom = strlen(nom_a_afficher);
                int largeur_contenu = largeur_visuelle - 1;

                if (longueur_nom > largeur_contenu) {
                    longueur_nom = largeur_contenu;
                }
                
                int caracteres_restants = largeur_contenu - longueur_nom;
                int padding_gauche = caracteres_restants / 2;
                int padding_droite = caracteres_restants - padding_gauche;

                printf("%s", couleurs_processus[couleur_idx]);
                for(int k=0; k < padding_gauche; k++) printf(" ");
                printf("%.*s", longueur_nom, nom_a_afficher);
                for(int k=0; k < padding_droite; k++) printf(" ");
                printf("%s", COULEUR_RESET);
            } else {
                for(int k=0; k < largeur_visuelle - 1; k++) printf("-");
            }
            i++;
        }
        printf("|\n");

        int current_pos = 0;
        for (int j = segment_initial; j <= i; j++) {
            char time_str[20];
            sprintf(time_str, "%d", transitions_uniques[j]);
            int len = strlen(time_str);

            int width = 0;
            if (j > segment_initial) {
                width = (transitions_uniques[j] - transitions_uniques[j-1]) * 3 + 1;
            }

            if (j == segment_initial) {
                 printf("%s", time_str);
                 current_pos += len;
            } else {
                int spaces = width - len;
                for (int k = 0; k < spaces; k++) {
                    printf(" ");
                }
                printf("%s", time_str);
                current_pos += width;
            }
        }
        printf("\n\n");
    }
}


void afficher_resultats(Processus tableau_processus[], int nombre_processus) {
    printf("\n======================================================\n");
    printf("===== Résultat de l'Ordonnancement des Processus =====\n");
    printf("======================================================\n");
    printf("\n===== Temps d'Arrivée et de Sortie =====\n");
    printf("%-15s | %-10s | %-10s\n", "Processus", "Arrivee", "Sortie");
    printf("-----------------------------------------\n");
    
    int temps_total = 0;
    for (int i = 0; i < nombre_processus; i++) {
        printf("%-15s | %-10d | %-10d\n", 
               tableau_processus[i].nom, 
               tableau_processus[i].arrivee, 
               tableau_processus[i].temps_sortie);
        if (tableau_processus[i].temps_sortie > temps_total) {
            temps_total = tableau_processus[i].temps_sortie;
        }
    }
    
    afficher_diagramme_gantt(tableau_processus, nombre_processus);
}