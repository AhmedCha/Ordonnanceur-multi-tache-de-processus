GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

all: main $(POL_LIBS)

build/politiques:
	mkdir -p build/politiques

MAIN_OBJS = main.o affichage.o interface_graphique.o resultats_window.o

main: $(MAIN_OBJS)
	gcc -Wall -Wextra -o $@ $^ -ldl $(GTK_LIBS) -lm

main.o: main.c processus.h
	gcc -Wall -Wextra -c $< $(GTK_CFLAGS)

affichage.o: affichage.c processus.h
	gcc -Wall -Wextra -c $<

interface_graphique.o: interface_graphique.c processus.h
	gcc -Wall -Wextra -c $< $(GTK_CFLAGS)

resultats_window.o: resultats_window.c processus.h
	gcc -Wall -Wextra -c $< $(GTK_CFLAGS)

build/politiques/%.so: politiques/%.c processus.h | build/politiques
	gcc -Wall -Wextra -fPIC -shared -o $@ $< -I..

clean:
	rm -f main $(MAIN_OBJS)
	rm -rf build
	rm -f politiques/*.o

re: clean all

.PHONY: all clean re