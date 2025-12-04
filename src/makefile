GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

all: main $(POL_LIBS)

build/politiques:
	mkdir -p build/politiques

MAIN_OBJS = main.o affichage.o Interface_graphique.o

main: $(MAIN_OBJS)
	gcc -Wall -Wextra -fPIC -o $@ $^ -ldl $(GTK_LIBS)

main.o: main.c processus.h
	gcc -Wall -Wextra -fPIC -c $< $(GTK_CFLAGS)

affichage.o: affichage.c processus.h
	gcc -Wall -Wextra -fPIC -c $<

Interface_graphique.o: Interface_graphique.c processus.h
	gcc -Wall -Wextra -fPIC -c $< $(GTK_CFLAGS)

build/politiques/%.so: politiques/%.c processus.h $(MAIN_OBJS) | build/politiques
	gcc -Wall -Wextra -fPIC -shared -o $@ $< $(MAIN_OBJS) $(GTK_LIBS)

clean:
	rm -f main $(MAIN_OBJS)
	rm -rf build
	rm -f politiques/*.o  # AJOUT pour nettoyer mlfq.o

re: clean all

.PHONY: all clean re
