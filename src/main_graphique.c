// main_graphique.c - VERSION FINALE PARFAITE (comme tu veux)
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
gboolean algo_lance = FALSE;

double couleurs[6][3] = {
    {0.1, 0.5, 1.0}, {0.0, 0.8, 0.4}, {1.0, 0.3, 0.3},
    {0.9, 0.6, 0.0}, {0.8, 0.2, 0.9}, {0.0, 0.7, 0.9}
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
    if (temps_actuel > temps_max + 10) {
        animation_en_cours = FALSE;
        return FALSE;
    }
    gtk_widget_queue_draw(drawing_area);
    return TRUE;
}

void lancer_algo(GtkWidget *w, gpointer data) {
    const char *algo = (const char*)data;
    char path[256];
    snprintf(path, sizeof(path), "./build/politiques/%s.so", algo);
    void *h = dlopen(path, RTLD_LAZY);
    if (!h) return;

    void (*f)(Processus[], int) = dlsym(h, "ordonnancer");
    if (!f) { dlclose(h); return; }

    for (int i = 0; i < nprocs; i++) {
        procs[i].restant = procs[i].duree;
        procs[i].nb_segments = 0;
        procs[i].temps_sortie = -1;
    }

    temps_actuel = 0;
    temps_max = 0;

    f(procs, nprocs);

    // Fusion des segments contigus (comme tu l’avais)
    for (int i = 0; i < nprocs; i++) {
        int j = 0;
        while (j < procs[i].nb_segments - 1) {
            if (procs[i].gantt[j].fin == procs[i].gantt[j+1].debut) {
                procs[i].gantt[j].fin = procs[i].gantt[j+1].fin;
                for (int k = j+1; k < procs[i].nb_segments-1; k++)
                    procs[i].gantt[k] = procs[i].gantt[k+1];
                procs[i].nb_segments--;
            } else j++;
        }
    }

    for (int i = 0; i < nprocs; i++)
        if (procs[i].temps_sortie > temps_max)
            temps_max = procs[i].temps_sortie;

    algo_lance = TRUE;
    animation_en_cours = TRUE;
    g_timeout_add(700, animer, NULL);
    gtk_widget_queue_draw(drawing_area);
    dlclose(h);
}

static gboolean dessiner_gantt(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    if (nprocs == 0 || !algo_lance) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_set_font_size(cr, 40);
        cairo_move_to(cr, width/2 - 300, height/2);
        cairo_show_text(cr, "Choisissez un algorithme pour lancer la simulation");
        return FALSE;
    }

    double echelle_x = 50.0;
    double y0 = 100;
    double hauteur_barre = 70;
    double espacement = 100;

    int temps_aff = animation_en_cours ? temps_actuel : temps_max;

    // === Processus + barres (avec bordures noires) ===
    for (int i = 0; i < nprocs; i++) {
        double y = y0 + i * espacement;

        // Nom du processus
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 32);
        cairo_move_to(cr, 50, y + 50);
        cairo_show_text(cr, procs[i].nom);

        // === BARRES AVEC BORDURE NOIRE ===
        int debut_courant = -1;
        int fin_courant = -1;

        for (int s = 0; s < procs[i].nb_segments; s++) {
            int d = procs[i].gantt[s].debut;
            int f = procs[i].gantt[s].fin;
            if (d >= temps_aff) break;
            if (f > temps_aff) f = temps_aff;

            if (debut_courant == -1) {
                debut_courant = d;
                fin_courant = f;
            } else if (d <= fin_courant) {
                if (f > fin_courant) fin_courant = f;
            } else {
                double x1 = 200 + debut_courant * echelle_x;
                double w = (fin_courant - debut_courant) * echelle_x;

                // Couleur
                cairo_set_source_rgb(cr, couleurs[i % 6][0], couleurs[i % 6][1], couleurs[i % 6][2]);
                cairo_rectangle(cr, x1, y + 10, w, hauteur_barre);
                cairo_fill(cr);

                // Bordure noire (gardée)
                cairo_set_source_rgb(cr, 0, 0, 0);
                cairo_set_line_width(cr, 3);
                cairo_rectangle(cr, x1, y + 10, w, hauteur_barre);
                cairo_stroke(cr);

                // Nom centré
                cairo_set_source_rgb(cr, 1, 1, 1);
                cairo_set_font_size(cr, 24);
                char txt[50];
                snprintf(txt, sizeof(txt), "%s", procs[i].nom);
                cairo_text_extents_t extents;
                cairo_text_extents(cr, txt, &extents);
                cairo_move_to(cr, x1 + (w - extents.width)/2, y + 45);
                cairo_show_text(cr, txt);

                debut_courant = d;
                fin_courant = f;
            }
        }

        // Dernier bloc
        if (debut_courant != -1) {
            double x1 = 200 + debut_courant * echelle_x;
            double w = (fin_courant - debut_courant) * echelle_x;

            cairo_set_source_rgb(cr, couleurs[i % 6][0], couleurs[i % 6][1], couleurs[i % 6][2]);
            cairo_rectangle(cr, x1, y + 10, w, hauteur_barre);
            cairo_fill(cr);

            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_set_line_width(cr, 3);
            cairo_rectangle(cr, x1, y + 10, w, hauteur_barre);
            cairo_stroke(cr);

            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_set_font_size(cr, 24);
            char txt[50];
            snprintf(txt, sizeof(txt), "%s", procs[i].nom);
            cairo_text_extents_t extents;
            cairo_text_extents(cr, txt, &extents);
            cairo_move_to(cr, x1 + (w - extents.width)/2, y + 45);
            cairo_show_text(cr, txt);
        }
    }

    // === AXE TEMPS EN BAS – TOUS LES CHIFFRES 0 1 2 3 ... ===
    double y_axe = y0 + nprocs * espacement + 50;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, 200, y_axe);
    cairo_line_to(cr, 200 + (temps_max + 10) * echelle_x, y_axe);
    cairo_stroke(cr);

    for (int t = 0; t <= temps_max + 5; t++) {
        double x = 200 + t * echelle_x;
        if (x > width - 100) break;

        cairo_move_to(cr, x, y_axe - 20);
        cairo_line_to(cr, x, y_axe + 20);
        cairo_stroke(cr);

        char buf[10];
        snprintf(buf, sizeof(buf), "%d", t);
        cairo_set_font_size(cr, 20);
        cairo_move_to(cr, x - 15, y_axe + 45);
        cairo_show_text(cr, buf);
    }

    return FALSE;
}

int main(int argc, char **argv) {
    if (argc < 2) { printf("Usage: %s <fichier.txt>\n", argv[0]); return 1; }
    lire_processus(argv[1]);

    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Ordonnanceur ESI - Dr. Najar Yousra");
    gtk_window_set_default_size(GTK_WINDOW(win), 1900, 900);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(win), box);

    GtkWidget *btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start(GTK_BOX(box), btns, FALSE, FALSE, 20);

    const char *algos[] = {"fifo", "priorite", "round_robin", "mlfq", "aging", NULL};
    for (int i = 0; algos[i]; i++) {
        GtkWidget *b = gtk_button_new_with_label(algos[i]);
        gtk_widget_set_size_request(b, 300, 100);
        g_signal_connect(b, "clicked", G_CALLBACK(lancer_algo), (gpointer)algos[i]);
        gtk_box_pack_start(GTK_BOX(btns), b, FALSE, FALSE, 15);
    }

    drawing_area = gtk_drawing_area_new();
    g_signal_connect(drawing_area, "draw", G_CALLBACK(dessiner_gantt), NULL);
    gtk_box_pack_start(GTK_BOX(box), drawing_area, TRUE, TRUE, 0);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}