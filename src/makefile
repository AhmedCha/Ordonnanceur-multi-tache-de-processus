# MAKEFILE FINAL – TERMINAL 100% CLEAN
CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I.
GTKFLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
LDFLAGS = -ldl -lm

POL_DIR = politiques
BUILD_DIR = build/politiques

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.so: $(POL_DIR)/%.c processus.h | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -shared -o $@ $< -w

main_graphique: main_graphique.o
	@$(CC) -o $@ $^ $(GTKFLAGS) $(LDFLAGS) -w

main_graphique.o: main_graphique.c processus.h
	@$(CC) $(CFLAGS) $(GTKFLAGS) -c $< -o $@ -w

all: main_graphique $(BUILD_DIR)/fifo.so $(BUILD_DIR)/priorite.so $(BUILD_DIR)/round_robin.so $(BUILD_DIR)/mlfq.so $(BUILD_DIR)/aging.so

run: all
	@./main_graphique processus.txt &

clean:
	@rm -f main_graphique *.o
	@rm -rf build

.PHONY: all clean run