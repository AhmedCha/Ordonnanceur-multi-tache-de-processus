makefile:
SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

all: main $(POL_LIBS)

build/politiques:
	mkdir -p build/politiques

MAIN_OBJS = main.o affichage.o

main: $(MAIN_OBJS)
	gcc -Wall -Wextra -fPIC -o $@ $^ -ldl

main.o: main.c processus.h
	gcc -Wall -Wextra -fPIC -c $<

affichage.o: affichage.c processus.h
	gcc -Wall -Wextra -fPIC -c $<

build/politiques/%.so: politiques/%.c processus.h | build/politiques
	gcc -Wall -Wextra -fPIC -shared -o $@ $<

clean:
	rm -f main $(MAIN_OBJS)
	rm -rf build
	rm -f politiques/*.o  # AJOUT pour nettoyer mlfq.o

re: clean all

.PHONY: all clean re
