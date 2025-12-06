# Makefile ULTIME – Mini Projet Ordonnancement ESI
# Tape juste : make run processus.txt

CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I.
GTKFLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
LDFLAGS = -ldl -lm

POL_DIR = politiques
BUILD_DIR = build/politiques

# Tous les .c dans politiques → tous les .so
POL_SRCS = $(wildcard $(POL_DIR)/*.c)
POL_LIBS = $(patsubst $(POL_DIR)/%.c,$(BUILD_DIR)/%.so,$(POL_SRCS))

all: main_graphique $(POL_LIBS)

main_graphique: main_graphique.o
	$(CC) -o $@ $^ $(GTKFLAGS) $(LDFLAGS)

main_graphique.o: main_graphique.c processus.h
	$(CC) $(CFLAGS) $(GTKFLAGS) -c $< -o $@

$(BUILD_DIR)/%.so: $(POL_DIR)/%.c processus.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -f main_graphique *.o
	rm -rf build

# LA COMMANDE QUE TU VAS UTILISER TOUS LES JOURS
run: all
	./main_graphique $(filter-out $@,$(MAKECMDGOALS))

# Permet de passer le fichier en argument
%::
	@:

.PHONY: all clean run
