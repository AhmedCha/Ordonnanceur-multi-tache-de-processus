// Interface_graphique.c — VERSION FINALE : DOUCE, MODERNE, SANS VIOLET
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <math.h>
#include "processus.h"

enum { COL_NOM, COL_ARRIVEE, COL_DUREE, COL_PRIORITE, NUM_COLS };

static GtkListStore *store;
static Processus processus_list[100];
static int num_processus = 0;
static GtkWidget *gantt_drawing_area;
static GtkWidget *algorithm_combo;
static GtkWidget *quantum_spin_button;
int global_quantum = 4;

int temps_actuel = 0, temps_max = 0;
gboolean animation_en_cours = FALSE, algo_lance = FALSE;

// Palette douce et professionnelle (bleu ciel, vert d'eau, corail, beige)
double couleurs[6][3] = {
    {0.42, 0.70, 0.98},  // Bleu ciel
    {0.52, 0.85, 0.70},  // Vert menthe
    {0.98, 0.70, 0.70},  // Corail doux
    {1.00, 0.80, 0.60},  // Orange crème
    {0.80, 0.70, 0.98},  // Lavande très claire
    {0.65, 0.85, 0.95}   // Cyan doux
};

static gboolean animer(gpointer data) {
    temps_actuel += 2;
    if (temps_actuel > temps_max + 20) {
        animation_en_cours = FALSE;
        return FALSE;
    }
    gtk_widget_queue_draw(gantt_drawing_area);
    return TRUE;
}

gboolean draw_gantt_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width  = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    if (num_processus == 0 || !algo_lance) return FALSE;

    double marge_gauche = 160, marge_droite = 100, marge_haut = 80, marge_bas = 120;
    double zone_x = width - marge_gauche - marge_droite;
    double zone_y = height - marge_haut - marge_bas;

    double echelle_x = zone_x / (temps_max + 25);
    double hauteur_barre = 90;
    double espacement = zone_y / (num_processus + 1);
    if (espacement < 120) { espacement = 120; hauteur_barre = 80; }

    double y0 = marge_haut + 40;
    int temps_aff = animation_en_cours ? temps_actuel : temps_max;

    for (int i = 0; i < num_processus; i++) {
        double y = y0 + i * espacement;

        cairo_set_source_rgb(cr, 0.1, 0.1, 0.15);
        cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 36);
        cairo_move_to(cr, 25, y + 55);
        cairo_show_text(cr, processus_list[i].nom);

        for (int s = 0; s < processus_list[i].nb_segments; s++) {
            if (processus_list[i].diagramme_gantt[s].debut > temps_aff) continue;
            double debut = marge_gauche + processus_list[i].diagramme_gantt[s].debut * echelle_x;
            double fin   = marge_gauche + fmin(processus_list[i].diagramme_gantt[s].fin, temps_aff + 1) * echelle_x;

            if (fin > debut) {
                cairo_set_source_rgba(cr, couleurs[i % 6][0], couleurs[i % 6][1], couleurs[i % 6][2], 0.94);
                cairo_rectangle(cr, debut, y + 12, fin - debut, hauteur_barre);
                cairo_fill(cr);

                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_set_line_width(cr, 3.5);
                cairo_rectangle(cr, debut, y + 12, fin - debut, hauteur_barre);
                cairo_stroke(cr);

                cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
                cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
                cairo_set_font_size(cr, 34);
                char txt[32];
                snprintf(txt, sizeof(txt), "%s", processus_list[i].nom);
                cairo_text_extents_t ext;
                cairo_text_extents(cr, txt, &ext);
                cairo_move_to(cr, debut + (fin - debut - ext.width) / 2, y + 62);
                cairo_show_text(cr, txt);
            }
        }
    }

    double y_axe = height - marge_bas + 50;
    cairo_set_source_rgb(cr, 0.15, 0.15, 0.15);
    cairo_set_line_width(cr, 7);
    cairo_move_to(cr, marge_gauche, y_axe);
    cairo_line_to(cr, width - marge_droite, y_axe);
    cairo_stroke(cr);

    for (int t = 0; t <= temps_max; t++) {
        double x = marge_gauche + t * echelle_x;
        if (x > width - marge_droite) break;

        cairo_set_line_width(cr, 3);
        cairo_move_to(cr, x, y_axe - 22);
        cairo_line_to(cr, x, y_axe + 22);
        cairo_stroke(cr);

        char buf[16];
        snprintf(buf, sizeof(buf), "%d", t);
        cairo_set_font_size(cr, 20);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, buf, &ext);
        cairo_move_to(cr, x - ext.width / 2, y_axe + 70);
        cairo_show_text(cr, buf);
    }

    return FALSE;
}

void cell_edited_callback(GtkCellRendererText *renderer,
                          gchar               *path_string,
                          gchar               *new_text,
                          gpointer             user_data)
{
    // SI L'UTILISATEUR ANNULE (ÉCHAP) OU CLIC SIMPLE → new_text EST VIDE → ON NE FAIT RIEN
    if (!new_text || new_text[0] == '\0') {
        return;  // C'EST LA LIGNE QUI SAUVE TOUT
    }

    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    if (!path) return;

    if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path)) {
        gtk_tree_path_free(path);
        return;
    }

    gint *indices = gtk_tree_path_get_indices(path);
    if (!indices) {
        gtk_tree_path_free(path);
        return;
    }
    int row = indices[0];
    gtk_tree_path_free(path);

    int col = GPOINTER_TO_INT(user_data);
    int value;

    switch (col) {
        case COL_NOM:
            strncpy(processus_list[row].nom, new_text, 19);
            processus_list[row].nom[19] = '\0';
            gtk_list_store_set(store, &iter, COL_NOM, processus_list[row].nom, -1);
            break;

        case COL_ARRIVEE:
            value = atoi(new_text);
            if (value >= 0) {
                processus_list[row].arrivee = value;
                gtk_list_store_set(store, &iter, COL_ARRIVEE, value, -1);
            }
            break;

        case COL_DUREE:
            value = atoi(new_text);
            if (value > 0) {
                processus_list[row].duree = value;
                processus_list[row].restant = value;
                gtk_list_store_set(store, &iter, COL_DUREE, value, -1);
            }
            break;

        case COL_PRIORITE:
            value = atoi(new_text);
            if (value >= 0) {
                processus_list[row].priorite = value;
                gtk_list_store_set(store, &iter, COL_PRIORITE, value, -1);
            }
            break;
    }
}

void populate_algorithms() {
    DIR *dir = opendir("build/politiques");
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strstr(entry->d_name, ".so") && entry->d_name[0] != '.') {
            // EXCLURE aging.so
            if (strcmp(entry->d_name, "aging.so") == 0) continue;
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), entry->d_name);
        }
    }
    closedir(dir);
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 0);
}
void on_quantum_changed(GtkSpinButton *spin_button, gpointer user_data G_GNUC_UNUSED) {
    global_quantum = gtk_spin_button_get_value_as_int(spin_button);
}

void on_algorithm_changed(GtkComboBox *widget, gpointer user_data G_GNUC_UNUSED) {
    gchar *active_algo = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    if (!active_algo) return;
    char lib_path[256];
    snprintf(lib_path, sizeof(lib_path), "build/politiques/%s", active_algo);
    g_free(active_algo);
    void *handle = dlopen(lib_path, RTLD_LAZY);
    gtk_widget_set_visible(quantum_spin_button, handle && dlsym(handle, "definir_quantum"));
    if (handle) dlclose(handle);
}

void run_scheduler_callback(GtkButton *button G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    gchar *active_lib = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(algorithm_combo));
    if (!active_lib) return;

    char lib_path[256];
    snprintf(lib_path, sizeof(lib_path), "build/politiques/%s", active_lib);
    g_free(active_lib);

    void *handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) return;

    void (*ordonnancer)(Processus[], int) = dlsym(handle, "ordonnancer");
    void (*definir_quantum)(int) = dlsym(handle, "definir_quantum");

   for (int i = 0; i < num_processus; i++) {
    processus_list[i].restant = processus_list[i].duree;  // <-- Ajoute cette ligne
    processus_list[i].nb_segments = 0;
    processus_list[i].temps_sortie = -1;
}

    if (definir_quantum) definir_quantum(global_quantum);
    if (ordonnancer) ordonnancer(processus_list, num_processus);

    temps_max = 0;
    for (int i = 0; i < num_processus; i++)
        if (processus_list[i].temps_sortie > temps_max)
            temps_max = processus_list[i].temps_sortie;

    algo_lance = TRUE;
    animation_en_cours = TRUE;
    temps_actuel = 0;
    g_timeout_add(200, animer, NULL);
    gtk_widget_queue_draw(gantt_drawing_area);

    dlclose(handle);
}

void launch_gui(int argc, char *argv[], const char* filename) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Ordonnanceur de Processus");
    gtk_window_set_default_size(GTK_WINDOW(window), 1600, 900);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // STYLE GLOBAL DOUX ET MODERNE — 100% SANS VIOLET
    GtkCssProvider *global_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(global_css,
        "window { "
        "   font-family: 'Segoe UI', sans-serif; "
        "   background: #f0f4f8; "                     /* Fond très clair pour contraste */
        "}"
        "button { "
        "   padding: 18px 38px; "
        "   font-size: 17px; "
        "   font-weight: bold; "
        "   border-radius: 24px; "
        "   margin: 12px; "
        "   background: #B0C4DE; "                     /* Ta couleur exacte */
        "   color: #2d3748; "
        "   border: none; "
        "   box-shadow: 0 6px 20px rgba(176,196,222,0.3); "
        "}"
        "button:hover { "
        "   background: #9fb5d6; "                     /* Un peu plus foncé au hover */
        "   transform: translateY(-4px); "
        "   box-shadow: 0 15px 35px rgba(176,196,222,0.5); "
        "}"
        "combobox, spinbutton { "
        "   font-size: 16px; "
        "   padding: 14px; "
        "   border-radius: 20px; "
        "   background: white; "
        "   border: 3px solid #B0C4DE; "               /* Bordure en #B0C4DE */
        "}"
        "combobox button { background: #B0C4DE; color: #2d3748; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(global_css),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_margin_start(top_bar, 25);
    gtk_widget_set_margin_end(top_bar, 25);
    gtk_widget_set_margin_top(top_bar, 20);
    gtk_widget_set_margin_bottom(top_bar, 15);
    gtk_box_pack_start(GTK_BOX(main_box), top_bar, FALSE, FALSE, 0);

    GtkWidget *open_btn = gtk_button_new_with_label("Ouvrir fichier");
    g_signal_connect(open_btn, "clicked", G_CALLBACK(open_file_callback), window);
    gtk_box_pack_start(GTK_BOX(top_bar), open_btn, FALSE, FALSE, 0);

    algorithm_combo = gtk_combo_box_text_new();
    populate_algorithms();
    g_signal_connect(algorithm_combo, "changed", G_CALLBACK(on_algorithm_changed), NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), algorithm_combo, TRUE, TRUE, 0);

    quantum_spin_button = gtk_spin_button_new_with_range(1, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(quantum_spin_button), 4);
    g_signal_connect(quantum_spin_button, "value-changed", G_CALLBACK(on_quantum_changed), NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), quantum_spin_button, FALSE, FALSE, 0);

    GtkWidget *run_btn = gtk_button_new_with_label("Lancer l'algorithme");
    g_signal_connect(run_btn, "clicked", G_CALLBACK(run_scheduler_callback), NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), run_btn, FALSE, FALSE, 0);

    gantt_drawing_area = gtk_drawing_area_new();
    g_signal_connect(gantt_drawing_area, "draw", G_CALLBACK(draw_gantt_callback), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), gantt_drawing_area, TRUE, TRUE, 0);

    // TABLEAU MAGNIFIQUE — COULEURS DOUCES
    GtkWidget *treeview = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
    GtkCssProvider *table_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(table_css,
        "treeview { "
        "   font-size: 18px; "
        "   background: white; "
        "   border-radius: 20px; "
        "   margin: 25px; "
        "   box-shadow: 0 8px 32px rgba(0,0,0,0.08); "
        "}"
        "treeview header button { "
        "   background: #f8f9fa; "          /* Gris très clair */
        "   color: #495057; "
        "   font-weight: bold; "
        "   padding: 18px; "
        "   font-size: 18px; "
        "   border-bottom: 2px solid #dee2e6; "
        "}"
        "treeview row:nth-child(even) { background: #f8f9fa; }"
        "treeview row:nth-child(odd)  { background: white; }"
        "treeview row:hover { background: #e9ecef; }"
        "treeview cell { padding: 18px; text-align: center; font-weight: 600; }",
        -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(treeview),
                                   GTK_STYLE_PROVIDER(table_css),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    const char *titles[] = {"Nom", "Arrivée", "Durée", "Priorité"};
    const char *colors[] = {"#0ea5e9", "#38bdf8", "#f472b6", "#84cc16"}; // Bleu, Cyan, Rose, Vert

    for (int i = 0; i < NUM_COLS; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer,
                     "editable", TRUE,
                     "weight", PANGO_WEIGHT_BOLD,
                     "foreground", colors[i],
                     "xalign", 0.5,
                     NULL);
        g_signal_connect(renderer, "edited", G_CALLBACK(cell_edited_callback), GINT_TO_POINTER(i));
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
            titles[i], renderer, "text", i, NULL);
        gtk_tree_view_column_set_alignment(col, 0.5);
        gtk_tree_view_column_set_expand(col, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
    }

    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    // g_object_unref(store);

    GtkWidget *tree_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(tree_scroll), treeview);
    gtk_widget_set_size_request(tree_scroll, -1, 240);
    gtk_box_pack_start(GTK_BOX(main_box), tree_scroll, FALSE, FALSE, 0);

    if (filename) load_file(filename);

    gtk_widget_show_all(window);
    gtk_main();
}