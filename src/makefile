# MAKEFILE PARFAIT – OBLIGE LE FICHIER EN PARAMÈTRE (comme la prof veut)
CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I.
GTKFLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
LDFLAGS = -ldl -lm

POL_DIR = politiques
BUILD_DIR = build/politiques

# Prend le fichier passé après "run"
FILE = $(firstword $(filter-out $@,$(MAKECMDGOALS)))

# Si aucun fichier → erreur claire
ifeq ($(FILE),)
$(error Erreur : Tu dois passer le fichier ! Exemple : make run processus.txt)
endif

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.so: $(POL_DIR)/%.c processus.h | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -shared -o $@ $< -w

main_graphique: main_graphique.o
	@$(CC) -o $@ $^ $(GTKFLAGS) $(LDFLAGS) -w

main_graphique.o: main_graphique.c processus.h
	@$(CC) $(CFLAGS) $(GTKFLAGS) -c $< -o $@ -w

all: main_graphique $(BUILD_DIR)/fifo.so $(BUILD_DIR)/priorite.so $(BUILD_DIR)/round_robin.so $(BUILD_DIR)/mlfq.so $(BUILD_DIR)/aging.so

# OBLIGE À PASSER LE FICHIER
run: all
	@if [ ! -f "$(FILE)" ]; then \
		echo "ERREUR : Le fichier '$(FILE)' n'existe pas !"; \
		echo "Utilise : make run processus.txt"; \
		exit 1; \
	fi
	@./main_graphique $(FILE) &

clean:
	@rm -f main_graphique *.o
	@rm -rf build

# Ignore les arguments qui ne sont pas des cibles (important !)
%::
	@:

.PHONY: all clean run
