SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

# Correction: tous les .o des fichiers src/
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: main $(POL_LIBS)

build/politiques:
	mkdir -p build/politiques

main: $(OBJS)
	gcc -Wall -Wextra -fPIC -o $@ $^ -ldl

# Règle générique pour les .o
%.o: %.c processus.h
	gcc -Wall -Wextra -fPIC -c -o $@ $<

# Correction: ajout de processus.h comme dépendance
build/politiques/%.so: politiques/%.c processus.h | build/politiques
	gcc -Wall -Wextra -fPIC -shared -o $@ $<

clean:
	rm -f main $(OBJS)
	rm -rf build

re: clean all

.PHONY: all clean re