#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <math.h>
#include "processus.h"

Processus procs[100];
int nprocs = 0;
GtkWidget *drawing_area;
int temps_actuel = 0;
int temps_max = 0;
gboolean animation_en_cours = FALSE;
gboolean algo_lance = FALSE;   // ← NOUVEAU : savoir si un algo a été lancé

double couleurs[][3] = {
    {0.1, 0.4, 0.9}, {0.2, 0.7, 0.3}, {0.9, 0.3, 0.3},
    {0.7, 0.2, 0.8}, {1.0, 0.6, 0.0}, {0.0, 0.7, 0.8}
};

void lire_processus(const char *f) {
    FILE *fp = fopen(f, "r");
    if (!fp) return;
    char ligne[256];
    nprocs = 0;
    while (fgets(ligne, sizeof(ligne), fp) && nprocs < 100) {
        if (ligne[0] == '#' || ligne[0] == '\n') continue;
        sscanf(ligne, "%s %d %d %d", procs[nprocs].nom, &procs[nprocs].arrivee,
               &procs[nprocs].duree, &procs[nprocs].priorite);
        procs[nprocs].restant = procs[nprocs].duree;
        procs[nprocs].nb_segments = 0;
        procs[nprocs].temps_sortie = -1;
        nprocs++;
    }
    fclose(fp);
}

static gboolean animer(gpointer data) {
    temps_actuel++;
    if (temps_actuel > temps_max) {
        animation_en_cours = FALSE;
        return FALSE;
    }
    gtk_widget_queue_draw(drawing_area);
    return TRUE;
}

void lancer_algo(GtkWidget *w, gpointer data) {
    const char *algo = (const char*)data;

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcc -fPIC -shared -o build/politiques/%s.so politiques/%s.c -w 2>/dev/null", algo, algo);
    system("mkdir -p build/politiques 2>/dev/null");
    system(cmd);

    char path[256];
    snprintf(path, sizeof(path), "./build/politiques/%s.so", algo);
    void *h = dlopen(path, RTLD_LAZY);
    if (!h) return;

    void (*f)(Processus[], int) = dlsym(h, "ordonnancer");
    if (f) {
        for (int i = 0; i < nprocs; i++) {
            procs[i].restant = procs[i].duree;
            procs[i].nb_segments = 0;
            procs[i].temps_sortie = -1;
        }
        temps_actuel = 0;
        temps_max = 0;
        f(procs, nprocs);

        for (int i = 0; i < nprocs; i++)
            if (procs[i].temps_sortie > temps_max)
                temps_max = procs[i].temps_sortie;
        if (temps_max == 0) temps_max = 50;

        algo_lance = TRUE;           // ← On active l’affichage
        animation_en_cours = TRUE;
        g_timeout_add(600, animer, NULL);
        gtk_widget_queue_draw(drawing_area);
    }
    dlclose(h);
}

static gboolean dessiner_gantt(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    if (nprocs == 0) return FALSE;

    double echelle_x = 32.0;
    double y0 = 120;
    double hauteur_ligne = 80;

    // Afficher les noms des processus (toujours visible)
    for (int i = 0; i < nprocs; i++) {
        double y = y0 + i * hauteur_ligne;
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_font_size(cr, 24);
        cairo_select_font_face(cr, "Liberation Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_move_to(cr, 30, y + 10);
        cairo_show_text(cr, procs[i].nom);
    }

    // UNIQUEMENT SI UN ALGO A ÉTÉ LANCÉ → on affiche tout
    if (algo_lance) {
        // Barres colorées
        for (int i = 0; i < nprocs; i++) {
            double y = y0 + i * hauteur_ligne;
            for (int s = 0; s < procs[i].nb_segments; s++) {
                double debut = 150 + procs[i].gantt[s].debut * echelle_x;
                double fin = 150 + fmin(procs[i].gantt[s].fin, temps_actuel + 1) * echelle_x;
                if (fin > debut) {
                    cairo_set_source_rgb(cr, couleurs[i % 6][0], couleurs[i % 6][1], couleurs[i % 6][2]);
                    cairo_rectangle(cr, debut, y - 30, fin - debut, 60);
                    cairo_fill(cr);

                    cairo_set_source_rgb(cr, 1, 1, 1);
                    cairo_set_font_size(cr, 20);
                    cairo_move_to(cr, debut + 15, y + 5);
                    cairo_show_text(cr, procs[i].nom);
                }
            }
        }

        // Axe temps en bas
        double y_axe = y0 + nprocs * hauteur_ligne + 40;
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 2);
        cairo_move_to(cr, 150, y_axe);
        cairo_line_to(cr, 150 + (temps_max + 5) * echelle_x, y_axe);
        cairo_stroke(cr);

        for (int t = 0; t <= temps_max + 2; t++) {
            double x = 150 + t * echelle_x;
            cairo_move_to(cr, x, y_axe - 8);
            cairo_line_to(cr, x, y_axe + 8);
            cairo_stroke(cr);

            char txt[16];
            snprintf(txt, sizeof(txt), "%d", t);
            cairo_set_font_size(cr, 16);
            cairo_move_to(cr, x - 10, y_axe + 28);
            cairo_show_text(cr, txt);
        }
    }

    return FALSE;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    lire_processus(argv[1]);

    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Ordonnanceur ESI - Gantt Pro");
    gtk_window_set_default_size(GTK_WINDOW(win), 1700, 900);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(win), box);

    GtkWidget *btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_pack_start(GTK_BOX(box), btns, FALSE, FALSE, 20);

    const char *algos[] = {"fifo", "priorite", "round_robin", "mlfq", "aging", NULL};
    for (int i = 0; algos[i]; i++) {
        GtkWidget *b = gtk_button_new_with_label(algos[i]);
        gtk_widget_set_size_request(b, 240, 90);
        g_signal_connect(b, "clicked", G_CALLBACK(lancer_algo), (gpointer)algos[i]);
        gtk_box_pack_start(GTK_BOX(btns), b, FALSE, FALSE, 10);
    }

    drawing_area = gtk_drawing_area_new();
    g_signal_connect(drawing_area, "draw", G_CALLBACK(dessiner_gantt), NULL);
    gtk_box_pack_start(GTK_BOX(box), drawing_area, TRUE, TRUE, 0);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}