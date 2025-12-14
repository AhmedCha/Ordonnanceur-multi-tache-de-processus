
#include "../processus.h"
#include <stdio.h>

#define QUANTUM 2
#define SEUIL_AGING 8
#define PRIORITE_MAX 5

typedef struct {
    int dernier_reveil;
} AgingData;

static AgingData aging_data[100];

#define PRIORITE_OFFSET 50
static int rr_cursor[PRIORITE_MAX + PRIORITE_OFFSET + 10] = {0};

void initialiser_aging(Processus processus[], int nombre) {
    for (int i = 0; i < nombre; i++) {
        aging_data[i].dernier_reveil = processus[i].arrivee;
    }
    for (int i = 0; i < PRIORITE_MAX + PRIORITE_OFFSET + 10; i++) {
        rr_cursor[i] = 0;
    }
}

void appliquer_aging(Processus processus[], int nombre, int temps) {
    for (int i = 0; i < nombre; i++) {
        if (processus[i].restant > 0 && processus[i].arrivee <= temps) {
            int attente = temps - aging_data[i].dernier_reveil;
            if (attente >= SEUIL_AGING && processus[i].priorite < PRIORITE_MAX) {
                int ancienne = processus[i].priorite;
                processus[i].priorite++;
                printf("[AGING t=%d] %s boosté %d → %d\n", temps, processus[i].nom, ancienne, processus[i].priorite);
                aging_data[i].dernier_reveil = temps;
                rr_cursor[processus[i].priorite + PRIORITE_OFFSET] = 0;
            }
        }
    }
}

int get_rr_index(int prio) {
    return prio + PRIORITE_OFFSET;
}

void ordonnancer(Processus processus[], int nombre) {
    printf("===== MLFQ Agressif + Aging (prio peut être négative !) =====\n\n");

    for (int i = 0; i < nombre; i++) {
        processus[i].restant = processus[i].duree;
        processus[i].nb_segments = 0;
        processus[i].temps_sortie = -1;
    }
    initialiser_aging(processus, nombre);

    int temps = 0;
    int termines = 0;
    int courant = -1;
    int quantum_use = 0;

    while (termines < nombre) {
        appliquer_aging(processus, nombre, temps);

        int prio_max = -9999;
        for (int i = 0; i < nombre; i++) {
            if (processus[i].restant > 0 && processus[i].arrivee <= temps) {
                if (processus[i].priorite > prio_max) {
                    prio_max = processus[i].priorite;
                }
            }
        }
        if (prio_max == -9999) { temps++; continue; }

        int candidats[100];
        int n = 0;
        for (int i = 0; i < nombre; i++) {
            if (processus[i].restant > 0 && processus[i].arrivee <= temps &&
                processus[i].priorite == prio_max) {
                candidats[n++] = i;
            }
        }

        int index = get_rr_index(prio_max);
        int choisi = candidats[rr_cursor[index] % n];
        rr_cursor[index] = (rr_cursor[index] + 1) % n;

        if (courant != choisi) {
            if (courant != -1) {
                Processus* p = &processus[courant];
                if (p->nb_segments > 0) {
                    p->diagramme_gantt[p->nb_segments - 1].fin = temps;
                }
                aging_data[courant].dernier_reveil = temps;

                if (processus[choisi].priorite > processus[courant].priorite) {
                    printf("[t=%d] PRÉEMPTION : %s → %s (prio %d > %d)\n",
                           temps, p->nom, processus[choisi].nom,
                           processus[choisi].priorite, processus[courant].priorite);
                } else {
                    printf("[t=%d] CHANGEMENT RR : %s → %s\n", temps, p->nom, processus[choisi].nom);
                }
            }

            courant = choisi;
            quantum_use = 0;

            Processus* p = &processus[courant];
            if (p->nb_segments < MAX_SEGMENTS_GANTT) {
                p->diagramme_gantt[p->nb_segments].debut = temps;
                p->diagramme_gantt[p->nb_segments].fin = temps + 1;
                p->nb_segments++;
            }
            printf("[t=%d] → %s (prio=%d, restant=%d)\n", temps, p->nom, p->priorite, p->restant);
        }

        processus[courant].restant--;
        quantum_use++;
        temps++;

        processus[courant].diagramme_gantt[processus[courant].nb_segments - 1].fin = temps;

        int ancienne_prio = processus[courant].priorite;
        processus[courant].priorite--;
        printf("[t=%d] DÉGRADATION : %s prio %d → %d\n", temps-1, processus[courant].nom, ancienne_prio, processus[courant].priorite);

        rr_cursor[get_rr_index(processus[courant].priorite)] = 0;

        if (processus[courant].restant == 0) {
            processus[courant].temps_sortie = temps;
            printf("[t=%d] TERMINÉ : %s\n", temps, processus[courant].nom);
            termines++;
            courant = -1;
            continue;
        }

        if (quantum_use >= QUANTUM) {
            printf("[t=%d] QUANTUM ÉPUISÉ → %s libéré\n", temps, processus[courant].nom);
            aging_data[courant].dernier_reveil = temps;
            courant = -1;
        }
    }

    printf("\n===== Ordonnancement terminé à t=%d =====\n", temps);
}