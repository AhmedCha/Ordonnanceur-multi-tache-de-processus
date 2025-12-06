// main_graphique.c – VERSION FINALE PROPRE – SANS MENU, TEMPS TOUS NUMÉROTÉS
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
    {0.11, 0.53, 0.98}, {0.35, 0.78, 0.60}, {0.95, 0.45, 0.45},
    {1.00, 0.70, 0.30}, {0.75, 0.50, 0.95}, {0.00, 0.70, 0.80}
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

void demander_quantum_et_lancer(GtkWidget *widget, gpointer data) {
    animation_en_cours = FALSE;
    algo_lance = FALSE;
    temps_actuel = 0;
    temps_max = 0;
    gtk_widget_queue_draw(drawing_area);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Round Robin - Quantum",
                                                   GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL,
                                                   "Annuler", GTK_RESPONSE_CANCEL,
                                                   "OK", GTK_RESPONSE_OK,
                                                   NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_box_pack_start(GTK_BOX(content), box, TRUE, TRUE, 20);

    gtk_box_pack_start(GTK_BOX(box), gtk_label_new("Quantum : "), FALSE, FALSE, 10);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "4");
    gtk_box_pack_start(GTK_BOX(box), entry, TRUE, TRUE, 10);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        int q = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
        if (q < 1) q = 4;

        void *handle = dlopen("./build/politiques/round_robin.so", RTLD_LAZY);
        if (!handle) return;

        void (*rr)(Processus[], int, int) = dlsym(handle, "ordonnancer");
        if (!rr) { dlclose(handle); return; }

        for (int i = 0; i < nprocs; i++) {
            procs[i].restant = procs[i].duree;
            procs[i].nb_segments = 0;
            procs[i].temps_sortie = -1;
        }

        rr(procs, nprocs, q);

        temps_max = 0;
        for (int i = 0; i < nprocs; i++)
            if (procs[i].temps_sortie > temps_max)
                temps_max = procs[i].temps_sortie;

        algo_lance = TRUE;
        animation_en_cours = TRUE;
        temps_actuel = 0;
        g_timeout_add(600, animer, NULL);
        gtk_widget_queue_draw(drawing_area);
        dlclose(handle);
    }
    gtk_widget_destroy(dialog);
}

void lancer_algo(GtkWidget *w, gpointer data) {
    animation_en_cours = FALSE;
    algo_lance = FALSE;
    temps_actuel = 0;
    temps_max = 0;
    gtk_widget_queue_draw(drawing_area);

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

    f(procs, nprocs);

    temps_max = 0;
    for (int i = 0; i < nprocs; i++)
        if (procs[i].temps_sortie > temps_max)
            temps_max = procs[i].temps_sortie;

    algo_lance = TRUE;
    animation_en_cours = TRUE;
    temps_actuel = 0;
    g_timeout_add(600, animer, NULL);
    gtk_widget_queue_draw(drawing_area);
    dlclose(h);
}

static gboolean dessiner_gantt(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 0.97, 0.97, 0.98);
    cairo_paint(cr);

    if (nprocs == 0 || !algo_lance) {
        cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 42);
        cairo_move_to(cr, width/2 - 280, height/2);
        cairo_show_text(cr, "Choisissez un algorithme");
        return FALSE;
    }

    double echelle_x = 45.0;
    double y0 = 140;
    double hauteur_barre = 75;
    double espacement = 110;

    int temps_aff = animation_en_cours ? temps_actuel : temps_max;

    for (int i = 0; i < nprocs; i++) {
        double y = y0 + i * espacement;

        cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 32);
        cairo_move_to(cr, 60, y + 50);
        cairo_show_text(cr, procs[i].nom);

        for (int s = 0; s < procs[i].nb_segments; s++) {
            if (procs[i].gantt[s].debut > temps_aff) continue;

            double debut = 220 + procs[i].gantt[s].debut * echelle_x;
            double fin = 220 + fmin(procs[i].gantt[s].fin, temps_aff + 1) * echelle_x;

            if (fin > debut) {
                cairo_set_source_rgba(cr, couleurs[i % 6][0], couleurs[i % 6][1], couleurs[i % 6][2], 0.9);
                cairo_rectangle(cr, debut, y + 12, fin - debut, hauteur_barre);
                cairo_fill(cr);

                cairo_set_source_rgb(cr, 0.15, 0.15, 0.15);
                cairo_set_line_width(cr, 2.5);
                cairo_rectangle(cr, debut, y + 12, fin - debut, hauteur_barre);
                cairo_stroke(cr);

                cairo_set_source_rgb(cr, 1, 1, 1);
                cairo_set_font_size(cr, 26);
                char txt[50];
                snprintf(txt, sizeof(txt), "%s", procs[i].nom);
                cairo_text_extents_t extents;
                cairo_text_extents(cr, txt, &extents);
                cairo_move_to(cr, debut + (fin - debut - extents.width)/2, y + 50);
                cairo_show_text(cr, txt);
            }
        }
    }

    // Axe temps – TOUS LES CHIFFRES (0 1 2 3 4 5 ...)
    double y_axe = y0 + nprocs * espacement + 60;
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 5);
    cairo_move_to(cr, 220, y_axe);
    cairo_line_to(cr, 220 + (temps_max + 15) * echelle_x, y_axe);
    cairo_stroke(cr);

    for (int t = 0; t <= temps_max + 10; t++) {
        double x = 220 + t * echelle_x;
        if (x > width - 100) break;

        cairo_set_line_width(cr, 2);
        cairo_move_to(cr, x, y_axe - 15);
        cairo_line_to(cr, x, y_axe + 15);
        cairo_stroke(cr);

        // TOUS LES CHIFFRES SONT AFFICHÉS
        char buf[10];
        snprintf(buf, sizeof(buf), "%d", t);
        cairo_set_font_size(cr, 18);
        cairo_move_to(cr, x - 12, y_axe + 40);
        cairo_show_text(cr, buf);
    }

    // Titre centré
    cairo_set_source_rgb(cr, 0.15, 0.4, 0.7);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 48);
    cairo_move_to(cr, width/2 - 300, 70);
    cairo_show_text(cr, "Diagramme de Gantt");

    return FALSE;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    lire_processus(argv[1]);

    gtk_init(&argc, &argv);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Diagramme de Gantt");
    gtk_window_set_default_size(GTK_WINDOW(win), 1800, 1000);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background-color: #f8f9fa; }"
        "button { background-color: #e3f2fd; color: #1565c0; font-weight: bold; padding: 15px; margin: 10px; border-radius: 12px; }"
        "button:hover { background-color: #bbdefb; }"
        , -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(win), main_box);

    // Gros boutons en bas (seuls visibles)
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box, FALSE, FALSE, 30);

    const char *algos[] = {"FIFO", "Priorite", "Round Robin", "MLFQ", "Aging", NULL};
    for (int i = 0; algos[i]; i++) {
        GtkWidget *b = gtk_button_new_with_label(algos[i]);
        gtk_widget_set_size_request(b, 240, 90);
        if (strcmp(algos[i], "Round Robin") == 0)
            g_signal_connect(b, "clicked", G_CALLBACK(demander_quantum_et_lancer), NULL);
        else
            g_signal_connect(b, "clicked", G_CALLBACK(lancer_algo), (gpointer)algos[i]);
        gtk_box_pack_start(GTK_BOX(btn_box), b, TRUE, TRUE, 10);
    }

    drawing_area = gtk_drawing_area_new();
    g_signal_connect(drawing_area, "draw", G_CALLBACK(dessiner_gantt), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), drawing_area, TRUE, TRUE, 0);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}