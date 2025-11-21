# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -ldl

# Source files and targets
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

# MLFQ static object (AJOUT)
MLFQ_OBJ = politiques/mlfq.o

# Default target
all: main $(POL_LIBS)

# Ensure build directories exist
build/politiques:
	mkdir -p build/politiques

# Build main program with MLFQ static (MODIFIÉ)
main: main.c $(MLFQ_OBJ)
	$(CC) $(CFLAGS) -o $@ $< $(MLFQ_OBJ) $(LDFLAGS)

# Build MLFQ static object (AJOUT)
$(MLFQ_OBJ): politiques/mlfq.c politiques/mlfq.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build each .so into build/politiques/ (EXCLURE mlfq si nécessaire)
build/politiques/%.so: politiques/%.c processus.h | build/politiques
	$(CC) $(CFLAGS) -shared -o $@ $<

# Clean (MODIFIÉ)
clean:
	rm -rf main
	rm -rf build
	rm -f politiques/*.o  # AJOUT pour nettoyer mlfq.o

# Rebuild
re: clean all

.PHONY: all clean re
