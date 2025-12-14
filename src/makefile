GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

all: main $(POL_LIBS)

# Crée le dossier des plugins
build/politiques:
	mkdir -p build/politiques

# Objets du programme principal (AJOUT de resultats_window.o)
MAIN_OBJS = main.o affichage.o Interface_graphique.o resultats_window.o

# Lien final : ON AJOUTE -lm (indispensable pour fmin)
main: $(MAIN_OBJS)
	gcc -Wall -Wextra -o $@ $^ -ldl $(GTK_LIBS) -lm

# Compilation des .o
main.o: main.c processus.h
	gcc -Wall -Wextra -c $< $(GTK_CFLAGS)

affichage.o: affichage.c processus.h
	gcc -Wall -Wextra -c $<

Interface_graphique.o: Interface_graphique.c processus.h
	gcc -Wall -Wextra -c $< $(GTK_CFLAGS)

# NOUVEAU: Compilation de resultats_window.o
resultats_window.o: resultats_window.c resultats_window.h processus.h
	gcc -Wall -Wextra -c $< $(GTK_CFLAGS)

# Compilation des plugins .so — AJOUT DE -I.. POUR INCLURE processus.h DU PARENT
build/politiques/%.so: politiques/%.c processus.h | build/politiques
	gcc -Wall -Wextra -fPIC -shared -o $@ $< -I..

# Nettoyage propre
clean:
	rm -f main $(MAIN_OBJS)
	rm -rf build
	rm -f politiques/*.o

re: clean all

.PHONY: all clean re