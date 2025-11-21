#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <termios.h>
#include <unistd.h>
#include "processus.h"
#include "politiques/mlfq.h"  // AJOUT pour MLFQ statique

int lire_processus(char *chemin_fichier, Processus tableau_processus[]) {
    FILE *fichier = fopen(chemin_fichier, "r");
    if (!fichier) { perror("Erreur fichier"); exit(1); }

    char ligne_lue[100];
    int nb_processus = 0;
    while (fgets(ligne_lue, sizeof(ligne_lue), fichier)) {
        if (ligne_lue[0] == '#')
            continue;
        
        int scanned = sscanf(ligne_lue, "%s %d %d %d",
                             tableau_processus[nb_processus].nom, 
                             &tableau_processus[nb_processus].arrivee, 
                             &tableau_processus[nb_processus].duree, 
                             &tableau_processus[nb_processus].priorite);
        
        if (scanned == 4)
            nb_processus++;
    }
    fclose(fichier);
    return nb_processus;
}

int getch() {
    struct termios oldt, newt;
    int caractere_lu;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    caractere_lu = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return caractere_lu;
}

int menu_interactif(char *options[], int nombre_options) {
    int index_selection = 0;
    int caractere_lu;

    while (1) {
        system("clear");
        printf("=== Ordonnanceurs disponibles ===\n\n");

        for (int i = 0; i < nombre_options; i++) {
            if (i == index_selection)
                printf(" > \033[32m%s\033[0m\n", options[i]);
            else
                printf("   %s\n", options[i]);
        }

        caractere_lu = getch();

        if (caractere_lu == '\n') {
            return index_selection;
        } else if (caractere_lu == 27) {
            getch();
            switch(getch()) {
                case 'A':
                    if (index_selection > 0) index_selection--;
                    break;
                case 'B':
                    if (index_selection < nombre_options - 1) index_selection++;
                    break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <fichier_configuration>\n", argv[0]);
        return 1;
    }

    // Options du menu
    char *options[] = {
        "FIFO (Dynamique)",
        "Round Robin (Dynamique)", 
        "Priorite Preemptive (Dynamique)",
        "MLFQ with Aging (Statique)",  // AJOUT
        "Quitter"
    };
    int nb_options = 5;

    // Charger les processus une fois
    Processus tableau_processus[100];
    int nb_processus = lire_processus(argv[1], tableau_processus);

    while (1) {
        int choix = menu_interactif(options, nb_options);

        if (choix == 4) { // Quitter
            printf("Fermeture de l'ordonnanceur.\n");
            break;
        }

        switch(choix) {
            case 0: // FIFO - Dynamique
            case 1: // Round Robin - Dynamique  
            case 2: // Priorite - Dynamique
                {
                    // Chargement dynamique comme avant
                    char* noms_politiques[] = {"fifo", "round_robin", "priorite"};
                    char chemin_lib[128];
                    snprintf(chemin_lib, sizeof(chemin_lib), "build/politiques/%s.so", noms_politiques[choix]);

                    void *bibliotheque = dlopen(chemin_lib, RTLD_LAZY);
                    if (!bibliotheque) {
                        fprintf(stderr, "Erreur chargement %s : %s\n", chemin_lib, dlerror());
                        break;
                    }

                    void (*ordonnancer)(Processus[], int);
                    *(void **)(&ordonnancer) = dlsym(bibliotheque, "ordonnancer");

                    char *error = dlerror();
                    if (error != NULL) {
                        fprintf(stderr, "Erreur symbole : %s\n", error);
                        dlclose(bibliotheque);
                        break;
                    }

                    printf("\nExécution de : %s\n", options[choix]);
                    ordonnancer(tableau_processus, nb_processus);
                    dlclose(bibliotheque);
                }
                break;

            case 3: // MLFQ - STATIQUE
                printf("\nExécution de : MLFQ with Aging\n");
                // Créer un tableau de pointeurs Processus* pour MLFQ
                Processus* processes[nb_processus];
                for (int i = 0; i < nb_processus; i++) {
                    processes[i] = &tableau_processus[i];
                    processes[i]->restant = processes[i]->duree; // Initialiser le temps restant
                }
                schedule_mlfq(processes, nb_processus);
                break;
        }
        
        printf("\nAppuyez sur une touche pour revenir au menu...\n");
        getch();
    }

    return 0;
}
