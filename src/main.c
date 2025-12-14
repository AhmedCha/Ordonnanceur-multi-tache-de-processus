#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <termios.h>
#include <unistd.h>
#include "processus.h"

void launch_gui(int argc, char *argv[], const char* filename);

int lire_processus(char *chemin_fichier, Processus tableau_processus[]) {
    FILE *fichier = fopen(chemin_fichier, "r");
    if (!fichier) { perror("Erreur fichier"); exit(1); }

    char ligne_lue[100];
    int nb_processus = 0;
    while (fgets(ligne_lue, sizeof(ligne_lue), fichier)) {
        if (ligne_lue[0] == '#') continue;
        
        int scanned = sscanf(ligne_lue, "%s %d %d %d",
                             tableau_processus[nb_processus].nom,
                             &tableau_processus[nb_processus].arrivee,
                             &tableau_processus[nb_processus].duree,
                             &tableau_processus[nb_processus].priorite);
        
        if (scanned == 4) nb_processus++;
    }
    fclose(fichier);
    return nb_processus;
}

int getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int menu_interactif(char *options[], char *display[], int nombre_options) {
    int index_selection = 0;
    int ch;

    while (1) {
        system("clear");
        printf("=== Ordonnanceurs disponibles ===\n\n");

        for (int i = 0; i < nombre_options; i++) {
            if (i == index_selection)
                printf(" > \033[32m%s\033[0m\n", display[i]);
            else{
                printf("   %s\n", display[i]);
            }
    }

        ch = getch();

        if (ch == '\n' || ch == '\r') {
            return index_selection;
        } else if (ch == 27) {
            if (getch() == '[') {
                switch(getch()) {
                    case 'A': if (index_selection > 0) index_selection--; break;
                    case 'B': if (index_selection < nombre_options - 1) index_selection++; break;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int run_gui = 0;
    const char* filename = NULL;

    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gui") == 0) {
            run_gui = 1;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }

    if (filename == NULL) {
        run_gui = 1;
    }

    if (run_gui) {
        launch_gui(argc, argv, filename);
        return 0;
    }

    if (filename == NULL) {
        printf("Usage (TUI): %s <fichier> [--tui]\n", argv[0]);
        printf("Usage (GUI): %s [--gui] [fichier]\n", argv[0]);
        return 1;
    }

    DIR *dir = opendir("build/politiques");
    if (!dir) {
        perror("Erreur ouverture build/politiques");
        return 1;
    }

    char *options[50];
    char *display[50];
    int nb_options = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".so") && entry->d_name[0] != '.') {
            options[nb_options] = strdup(entry->d_name);
            char *dot = strrchr(entry->d_name, '.');
            if (dot && strcmp(dot, ".so") == 0) {
                size_t len = dot - entry->d_name;
                display[nb_options] = malloc(len + 1);
                strncpy(display[nb_options], entry->d_name, len);
                display[nb_options][len] = '\0';
            } else {
                display[nb_options] = strdup(entry->d_name);
            }
            nb_options++;
        }
    }
    closedir(dir);

    if (nb_options == 0) {
        printf("Aucune politique trouvée !\n");
        return 1;
    }

    options[nb_options] = "Quitter";
    display[nb_options] = "Quitter";
    nb_options++;

    while (1) {
        Processus procs[100];
        int n = lire_processus((char*)filename, procs);

        int choix = menu_interactif(options, display, nb_options);
        if (strcmp(options[choix], "Quitter") == 0) break;

        char path[256];
        snprintf(path, sizeof(path), "build/politiques/%s", options[choix]);

        void *lib = dlopen(path, RTLD_LAZY);
        if (!lib) {
            fprintf(stderr, "Erreur: %s\n", dlerror());
            continue;
        }

        void (*ordonnancer)(Processus[], int) = dlsym(lib, "ordonnancer");
        void (*set_quantum)(int) = dlsym(lib, "definir_quantum");

        for (int i = 0; i < n; i++) {
            procs[i].restant = procs[i].duree;
            procs[i].nb_segments = 0;
            procs[i].temps_sortie = -1;
        }

        if (set_quantum) {
            int q;
            printf("Quantum: ");
            scanf("%d", &q);
            while(getchar() != '\n');
            set_quantum(q);
        }

        if (ordonnancer) ordonnancer(procs, n);

        afficher_resultats(procs, n);

        dlclose(lib);

        printf("\nAppuyez sur Entrée pour continuer...");
        getchar();
    }

    for (int i = 0; i < nb_options - 1; i++) {
        free(options[i]);
        free(display[i]);
    }

    return 0;
}