CFLAGS = -Wall -Wextra -fPIC

SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

all: main $(POL_LIBS)

build/politiques:
	mkdir -p build/politiques

MAIN_OBJS = main.o affichage.o

main: $(MAIN_OBJS)
	gcc $(CFLAGS) -o $@ $^ -ldl

main.o: main.c processus.h
	gcc $(CFLAGS) -c $<

affichage.o: affichage.c processus.h
	gcc $(CFLAGS) -c $<

build/politiques/%.so: politiques/%.c processus.h | build/politiques
	gcc $(CFLAGS) -shared -o $@ $<

clean:
	rm -f main $(MAIN_OBJS)
	rm -rf build

re: clean all

.PHONY: all clean re
