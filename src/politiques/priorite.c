#include <stdio.h>
#include "../processus.h"

void ordonnancer(Processus p[], int n) {
    printf("===== Ordonnancement par Priorité Préemptive =====\n");

    int temps = 0;
    int restant[n];
    int termine = 0;
    int courant = -1;
    int debut_seg = 0;

    for (int i = 0; i < n; i++) {
        restant[i] = p[i].duree;
        p[i].nb_segments = 0;
        p[i].temps_sortie = -1;
    }

    while (termine < n) {
        int meilleur = -1;
        int prio_max = -1;

        for (int i = 0; i < n; i++) {
            if (p[i].arrivee <= temps && restant[i] > 0 && p[i].priorite > prio_max) {
                prio_max = p[i].priorite;
                meilleur = i;
            }
        }

        if (meilleur == -1) {
            temps++;
            continue;
        }

        // Préemption : on ferme le segment du processus sortant
        if (courant != -1 && courant != meilleur) {
            int s = p[courant].nb_segments++;
            p[courant].gantt[s].debut = debut_seg;
            p[courant].gantt[s].fin = temps;
        }

        // Démarrage ou reprise
        if (courant != meilleur) {
            courant = meilleur;
            debut_seg = temps;
        }

        // Exécution
        restant[courant]--;
        temps++;

        if (restant[courant] == 0) {
            int s = p[courant].nb_segments++;
            p[courant].gantt[s].debut = debut_seg;
            p[courant].gantt[s].fin = temps;
            p[courant].temps_sortie = temps;
            termine++;
            courant = -1;
        }
    }

    // Fermer le dernier segment
    if (courant != -1) {
        int s = p[courant].nb_segments++;
        p[courant].gantt[s].debut = debut_seg;
        p[courant].gantt[s].fin = temps;
    }
}