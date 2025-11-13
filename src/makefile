# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -ldl

# Source files and targets
POL_SRCS = $(wildcard politiques/*.c)
POL_LIBS = $(patsubst politiques/%.c,build/politiques/%.so,$(POL_SRCS))

# Default target
all: main $(POL_LIBS)

# Ensure build directories exist
build/politiques:
	mkdir -p build/politiques

# Build main program
main: main.c
	gcc $(CFLAGS) -o $@ $< $(LDFLAGS)

# Build each .so into build/politiques/
build/politiques/%.so: politiques/%.c processus.h | build/politiques
	gcc $(CFLAGS) -shared -o $@ $<

# Clean
clean:
	rm -rf build main

# Rebuild
re: clean all

.PHONY: all clean re
