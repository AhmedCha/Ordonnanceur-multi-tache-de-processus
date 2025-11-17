#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <termios.h>
#include <unistd.h>
#include "processus.h"

int lire_fichier(char *nomfichier, Processus tab[]) {
    FILE *f = fopen(nomfichier, "r");
    if (!f) { perror("Erreur fichier"); exit(1); }

    char ligne[100];
    int n = 0;
    while (fgets(ligne, sizeof(ligne), f)) {
        if (ligne[0] == '#' || strlen(ligne) <= 1)
            continue;
        sscanf(ligne, "%s %d %d %d",
               tab[n].nom, &tab[n].arrivee, &tab[n].duree, &tab[n].priorite);
        n++;
    }
    fclose(f);
    return n;
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

int menu_interactif(char *options[], int count) {
    int selection = 0;   // Default selection
    char c;

    while (1) {
        system("clear");
        printf("=== Ordonnanceurs disponibles ===\n\n");

        for (int i = 0; i < count; i++) {
            if (i == selection)
                printf(" > \033[32m%s\033[0m\n", options[i]);  // highlighted
            else
                printf("   %s\n", options[i]);
        }

        c = getch();

        if (c == '\n') {           // ENTER
            return selection;
        } else if (c == 27) {      // ESC / arrow prefix
            getch();               // skip '['
            switch(getch()) {
                case 'A':          // UP
                    if (selection > 0) selection--;
                    break;
                case 'B':          // DOWN
                    if (selection < count - 1) selection++;
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

    Processus tab[100];
    int n = lire_fichier(argv[1], tab);

    DIR *dir = opendir("politiques");
    if (!dir) {
        perror("Erreur ouverture dossier politiques/");
        return 1;
    }

    struct dirent *entry;
    char *politiques[50];
    int count = 0;

    char *fifo_file = NULL; 

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".so")) {

            // Detect FIFO file first
            if (strncmp(entry->d_name, "fifo", 4) == 0) {
                fifo_file = strdup(entry->d_name);
            } else {
                politiques[count++] = strdup(entry->d_name);
            }
        }
    }
    closedir(dir);

    // Mettre FIFO par defaut s'il existe
    if (fifo_file != NULL) {
        for (int i = count; i > 0; i--) {
            politiques[i] = politiques[i - 1];
        }
        politiques[0] = fifo_file;
        count++;
    }


    if (count == 0) {
        printf("Aucune politique trouvée dans le dossier.\n");
        return 1;
    }

    int choix_index = menu_interactif(politiques, count);

    char chemin[128];
    snprintf(chemin, sizeof(chemin), "politiques/%s", politiques[choix_index]);

    void *lib = dlopen(chemin, RTLD_LAZY);
    if (!lib) {
        fprintf(stderr, "Erreur chargement %s : %s\n", chemin, dlerror());
        return 1;
    }

    void (*ordonnancer)(Processus[], int);
    *(void **)(&ordonnancer) = dlsym(lib, "ordonnancer");

    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Erreur symbole : %s\n", error);
        dlclose(lib);
        return 1;
    }

    printf("\nExécution de la politique : %s\n", politiques[choix_index]);
    ordonnancer(tab, n);

    dlclose(lib);
    return 0;
}
