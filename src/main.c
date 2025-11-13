#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <fichier_configuration>\n", argv[0]);
        return 1;
    }

    Processus tab[100];
    int n = lire_fichier(argv[1], tab);

    DIR *dir = opendir("build/politiques");
    if (!dir) {
        perror("Erreur ouverture dossier politiques/");
        return 1;
    }

    struct dirent *entry;
    char *politiques[50];
    int count = 0;

    printf("\n=== Ordonnanceurs disponibles ===\n");
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".so")) {  
            politiques[count] = strdup(entry->d_name);
            printf("%d. %s\n", count + 1, entry->d_name);
            count++;
        }
    }
    closedir(dir);

    if (count == 0) {
        printf("Aucune politique trouvée dans le dossier.\n");
        return 1;
    }

    int choix;
    printf("\nChoisissez une politique : ");
    scanf("%d", &choix);
    if (choix < 1 || choix > count) {
        printf("Choix invalide.\n");
        return 1;
    }

    char chemin[128];
    snprintf(chemin, sizeof(chemin), "build/politiques/%s", politiques[choix - 1]);
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

    printf("\nExécution de la politique : %s\n", politiques[choix - 1]);
    ordonnancer(tab, n);

    dlclose(lib);
    return 0;
}
