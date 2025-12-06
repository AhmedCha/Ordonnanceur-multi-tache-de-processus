#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <termios.h>
#include <unistd.h>
#include "processus.h"
#include <glib.h>

void launch_gui(int argc, char *argv[], const char* filename);

int lire_processus(char *chemin_fichier, Processus tableau_processus[]) {
    FILE *fichier = fopen(chemin_fichier, "r");
    if (!fichier) { perror("Erreur fichier"); exit(1); }

    char ligne_lue[100];
    int nb_processus = 0;
    while (fgets(ligne_lue, sizeof(ligne_lue), fichier)) {
        if (ligne_lue[0] == '#')
            continue;
        
        int scanned = sscanf(ligne_lue, "%s %d %d %d",
                             tableau_processus[nb_processus].nom, &tableau_processus[nb_processus].arrivee, &tableau_processus[nb_processus].duree, &tableau_processus[nb_processus].priorite);
        
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
    int run_gui = 1;
    const char* filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tui") == 0) {
            run_gui = 0;
        } else if (strcmp(argv[i], "--gui") == 0) {
            run_gui = 1;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }

    if (filename != NULL) {
        int gui_flag_present = 0;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--gui") == 0) {
                gui_flag_present = 1;
                break;
            }
        }
        if (!gui_flag_present) {
            run_gui = 0;
        }
    }
    
    if (run_gui) {
        g_setenv("GSETTINGS_BACKEND", "memory", TRUE);
        launch_gui(argc, argv, filename);
        return 0;
    }
    
    if (filename == NULL) {
        printf("Usage (TUI mode): %s <fichier_configuration> [--tui]\n", argv[0]);
        printf("Usage (GUI mode): %s [--gui] [fichier_configuration]\n", argv[0]);
        return 1;
    }

    DIR *repertoire = opendir("build/politiques");
    if (!repertoire) {
        perror("Erreur ouverture dossier build/politiques");
        return 1;
    }

    struct dirent *entree_dir;
    char *tableau_politiques[50];
    int nb_politiques = 0;
    char *nom_politique_fifo = NULL;

    while ((entree_dir = readdir(repertoire)) != NULL) {
        if (strstr(entree_dir->d_name, ".so")) {
            if (strncmp(entree_dir->d_name, "fifo", 4) == 0) {
                nom_politique_fifo = strdup(entree_dir->d_name);
            } else {
                tableau_politiques[nb_politiques++] = strdup(entree_dir->d_name);
            }
        }
    }
    closedir(repertoire);

    if (nom_politique_fifo != NULL) {
        for (int i = nb_politiques; i > 0; i--) {
            tableau_politiques[i] = tableau_politiques[i - 1];
        }
        tableau_politiques[0] = nom_politique_fifo;
        nb_politiques++;
    }
    if (nb_politiques == 0) {
        printf("Aucune politique trouvée dans le dossier.\n");
        return 1;
    }

    tableau_politiques[nb_politiques++] = "Quitter";

    while (1) {
        Processus tableau_processus[100];
        int nb_processus = lire_processus((char*)filename, tableau_processus);

        int index_selection = menu_interactif(tableau_politiques, nb_politiques);

        if (strcmp(tableau_politiques[index_selection], "Quitter") == 0) {
            printf("Fermeture de l'ordonnanceur.\n");
            break;
        }

        char chemin_lib[128];
        snprintf(chemin_lib, sizeof(chemin_lib), "build/politiques/%s", tableau_politiques[index_selection]);

        void *bibliotheque = dlopen(chemin_lib, RTLD_LAZY);
        if (!bibliotheque) {
            fprintf(stderr, "Erreur chargement %s : %s\n", chemin_lib, dlerror());
            continue;
        }

        void (*ordonnancer)(Processus[], int);
        *(void **)(&ordonnancer) = dlsym(bibliotheque, "ordonnancer");
        char *error = dlerror();
        if (error != NULL) {
            fprintf(stderr, "Erreur symbole : %s\n", error);
            dlclose(bibliotheque);
            continue;
        }

        if (ordonnancer == NULL) {
            dlclose(bibliotheque);
            continue;
        } 
        void (*definir_quantum)(int);
        *(void **)(&definir_quantum) = dlsym(bibliotheque, "definir_quantum");
        dlerror();
        if (definir_quantum != NULL) {
            int quantum_val = 0;
            printf("Cette politique utilise un quantum. Entrez la valeur du quantum: ");
            scanf("%d", &quantum_val);
            while(getchar() != '\n');
            definir_quantum(quantum_val);
        }

        ordonnancer(tableau_processus, nb_processus);

        afficher_resultats(tableau_processus, nb_processus);

        dlclose(bibliotheque);
        
        printf("\nAppuyez sur une touche pour revenir au menu...\n");
        getch();
    }

    for (int i = 0; i < nb_politiques - 1; i++) {
        free(tableau_politiques[i]);
    }

    return 0;
}