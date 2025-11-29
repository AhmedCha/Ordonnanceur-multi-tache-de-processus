#include <stdio.h>
#include "../processus.h"

#define MAX_PROCS 100
#define TAILLE_FILE 1000

void ordonnancer(Processus tab[], int N) {
    printf("===== PRIORITÉ PRÉEMPTIVE + RR(prio égale) =====\n");
    
    for(int i = 0; i < N; i++) {
        tab[i].restant = tab[i].duree;
        tab[i].nb_segments = 0;
        tab[i].temps_sortie = -1;
    }

    int temps = 0, finis = 0;
    int file_attente[TAILLE_FILE];
    int debut = 0, fin = 0;
    int current_prio_level = 0;  // ✅ RÈGLE 1

    while(finis < N) {
        // ✅ RÈGLE 1 : PRIORITÉ MAX + PRÉEMPTION
        int max_prio = 0;
        for(int i = 0; i < N; i++) {
            if(tab[i].restant > 0 && tab[i].arrivee <= temps)
                max_prio = (tab[i].priorite > max_prio) ? tab[i].priorite : max_prio;
        }
        if(max_prio == 0) { temps++; continue; }

        // ✅ PRÉEMPTION : new_max_prio > current_prio_level
        int new_max_prio = max_prio;
        if(new_max_prio > current_prio_level) {
            current_prio_level = new_max_prio;
            debut = fin = 0;  // Reset file
            printf("[PREEMPT t=%d] prio=%d\n", temps, new_max_prio);
        }

        // ✅ RÈGLE 3 : File RR pour processus à max_prio
        for(int i = 0; i < N; i++) {
            if(tab[i].restant > 0 && tab[i].arrivee <= temps && 
               tab[i].priorite == max_prio) {
                int en_file = 0;
                for(int pos = debut; pos != fin; pos = (pos + 1) % TAILLE_FILE) {
                    if(file_attente[pos] == i) { en_file = 1; break; }
                }
                if(!en_file) {
                    file_attente[fin] = i;
                    fin = (fin + 1) % TAILLE_FILE;
                }
            }
        }

        if(debut == fin) { temps++; continue; }

        // ✅ RÈGLE 4 : 1 UNITÉ SEULEMENT
        int i = file_attente[debut];
        debut = (debut + 1) % TAILLE_FILE;
        
        Processus* p = &tab[i];
        int debut_segment = temps;
        temps++;  // ✅ 1 unité
        p->restant--;  // ✅ RÈGLE 4
        
        if(p->nb_segments < MAX_SEGMENTS_GANTT) {
            p->diagramme_gantt[p->nb_segments].debut = debut_segment;
            p->diagramme_gantt[p->nb_segments].fin = temps;
            p->nb_segments++;
        }

        // ✅ RÈGLE 2 : Prio-- APRÈS 1 unité
        if(p->priorite > 1) {
            p->priorite--;
            printf("[t=%d] %s ↓prio=%d\n", debut_segment, p->nom, p->priorite);
        }

        // Remettre en file si pas fini (RR)
        if(p->restant > 0) {
            file_attente[fin] = i;
            fin = (fin + 1) % TAILLE_FILE;
        } else {
            p->temps_sortie = temps;
            finis++;
        }
    }
}
