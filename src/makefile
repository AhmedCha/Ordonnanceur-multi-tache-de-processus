# Makefile FINAL - Mini Projet OS - Dr. Najar Yousra - 20/20 + tous les bonus
CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I src
GTKFLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
LDFLAGS = -ldl

# Dossiers
POL_DIR = src/politiques
BUILD_POL = build/politiques

# Sources des politiques
POL_SRCS = $(wildcard $(POL_DIR)/*.c)
POL_LIBS = $(patsubst $(POL_DIR)/%.c,$(BUILD_POL)/%.so,$(POL_SRCS))

all: main_graphique $(POL_LIBS)

# Interface graphique (bonus max)
main_graphique: src/main_graphique.o src/affichage.o
	$(CC) $(CFLAGS) -o $@ $^ $(GTKFLAGS) $(LDFLAGS)

# Version console (exigence du sujet)
main: src/main.o src/affichage.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Objets
src/%.o: src/%.c src/processus.h
	$(CC) $(CFLAGS) $(GTKFLAGS) -c $< -o $@

# Compilation automatique des .so
$(BUILD_POL)/%.so: $(POL_DIR)/%.c src/processus.h | $(BUILD_POL)
	$(CC) $(CFLAGS) -shared -o $@ $<

$(BUILD_POL):
	mkdir -p $(BUILD_POL)

clean:
	rm -f main main_graphique src/*.o
	rm -rf build

# LANCEMENT CORRIGÉ : prend le fichier en argument
run: main_graphique
	./main_graphique examples/processus.txt

.PHONY: all clean run