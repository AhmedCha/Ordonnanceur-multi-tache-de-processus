CC          = gcc
CFLAGS      = -Wall -Wextra
GTK_CFLAGS  = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS    = $(shell pkg-config --libs gtk+-3.0)
LDFLAGS     = -ldl -lm

BUILD_DIR       = build
POL_BUILD_DIR   = $(BUILD_DIR)/politiques

SRCS = $(wildcard *.c)
POL_SRCS = $(wildcard politiques/*.c)

MAIN_OBJS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/affichage.o \
	$(BUILD_DIR)/interface_graphique.o \
	$(BUILD_DIR)/resultats_window.o

POL_LIBS = $(patsubst politiques/%.c,$(POL_BUILD_DIR)/%.so,$(POL_SRCS))
	
all: main $(POL_LIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(POL_BUILD_DIR):
	mkdir -p $(POL_BUILD_DIR)

main: $(MAIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)

$(BUILD_DIR)/main.o: main.c processus.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(BUILD_DIR)/affichage.o: affichage.c processus.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/interface_graphique.o: interface_graphique.c processus.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(BUILD_DIR)/resultats_window.o: resultats_window.c processus.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(POL_BUILD_DIR)/%.so: politiques/%.c processus.h | $(POL_BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< -I..

clean:
	rm -f main
	rm -rf $(BUILD_DIR)

re: clean all

.PHONY: all clean re