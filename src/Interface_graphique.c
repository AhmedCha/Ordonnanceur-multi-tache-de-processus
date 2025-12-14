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

static Processus processus_list[100];
static int num_processus = 0;
static int selected_row = -1;
static GtkWidget *gantt_drawing_area;
static GtkWidget *algorithm_combo;
static GtkWidget *quantum_spin_button;
static GtkWidget *table_grid;
static GtkWidget *table_scroll;
static GList *row_widgets = NULL;
int global_quantum = 4;

void build_table(void);

void afficher_fenetre_resultats(Processus processus_list[], int num_processus);

int temps_actuel = 0, temps_max = 0;
gboolean animation_en_cours = FALSE, algo_lance = FALSE;

double couleurs[6][3] = {
    {0.42, 0.70, 0.98},  
    {0.52, 0.85, 0.70},  
    {0.98, 0.70, 0.70},  
    {1.00, 0.80, 0.60},  
    {0.80, 0.70, 0.98},  
    {0.65, 0.85, 0.95}   
};

static gboolean animer(gpointer data G_GNUC_UNUSED) {
    temps_actuel += 2;
    if (temps_actuel > temps_max + 20) {
        animation_en_cours = FALSE;
        afficher_fenetre_resultats(processus_list, num_processus);
        return FALSE;
    }
    gtk_widget_queue_draw(gantt_drawing_area);
    return TRUE;
}

static void draw_gantt_bars(GtkWidget *widget, cairo_t *cr, double marge_gauche, double echelle_x, double hauteur_barre, double espacement, double y0, int temps_aff) {
    for (int i = 0; i < num_processus; i++) {
        double y = y0 + i * espacement + (espacement - hauteur_barre) / 2;

        cairo_set_source_rgb(cr, 0.1, 0.1, 0.15);
        cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, fmin(18, hauteur_barre * 0.5));
        cairo_move_to(cr, 25, y + hauteur_barre / 2 + 5);
        cairo_show_text(cr, processus_list[i].nom);

        for (int s = 0; s < processus_list[i].nb_segments; s++) {
            if (processus_list[i].diagramme_gantt[s].debut > temps_aff) continue;
            double debut = marge_gauche + processus_list[i].diagramme_gantt[s].debut * echelle_x;
            double fin   = marge_gauche + fmin(processus_list[i].diagramme_gantt[s].fin, temps_aff + 1) * echelle_x;

            if (fin > debut) {
                cairo_set_source_rgba(cr, couleurs[i % 6][0], couleurs[i % 6][1], couleurs[i % 6][2], 0.94);
                cairo_rectangle(cr, debut, y, fin - debut, hauteur_barre);
                cairo_fill(cr);

                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_set_line_width(cr, 2);
                cairo_rectangle(cr, debut, y, fin - debut, hauteur_barre);
                cairo_stroke(cr);

                cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
                cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
                cairo_set_font_size(cr, fmin(12, hauteur_barre * 0.4));
                char txt[32];
                snprintf(txt, sizeof(txt), "%s", processus_list[i].nom);
                cairo_text_extents_t ext;
                cairo_text_extents(cr, txt, &ext);
                if (fin - debut > ext.width + 10) {
                    cairo_move_to(cr, debut + (fin - debut - ext.width) / 2, y + hauteur_barre / 2 + 4);
                    cairo_show_text(cr, txt);
                }
            }
        }
    }
}

static void draw_gantt_scale(cairo_t *cr, double marge_gauche, double marge_droite, double y_axe, double echelle_x, int temps_max, int width) {
    cairo_set_source_rgb(cr, 0.15, 0.15, 0.15);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, marge_gauche, y_axe);
    cairo_line_to(cr, width - marge_droite, y_axe);
    cairo_stroke(cr);

    cairo_text_extents_t sample_ext;
    cairo_set_font_size(cr, 12);
    cairo_text_extents(cr, "0000", &sample_ext);
    double min_pixel_spacing = sample_ext.width + 20;

    int scale_interval = 1;
    while (scale_interval * echelle_x < min_pixel_spacing && scale_interval < temps_max) {
        scale_interval *= 2;
    }
    if (scale_interval < 1) scale_interval = 1;

    for (int t = 0; t <= temps_max; t++) {
        double x = marge_gauche + t * echelle_x;
        if (x > width - marge_droite) break;

        cairo_set_line_width(cr, 1.5);
        cairo_move_to(cr, x, y_axe - 8);
        cairo_line_to(cr, x, y_axe + 8);
        cairo_stroke(cr);

        if (t % scale_interval == 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", t);
            cairo_set_font_size(cr, 12);
            cairo_set_source_rgb(cr, 0.1, 0.1, 0.15);
            cairo_text_extents_t ext;
            cairo_text_extents(cr, buf, &ext);
            cairo_move_to(cr, x - ext.width / 2, y_axe + 25);
            cairo_show_text(cr, buf);
        }
    }
}

gboolean draw_gantt_callback(GtkWidget *widget, cairo_t *cr, gpointer data G_GNUC_UNUSED) {
    int width  = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    if (num_processus == 0 || !algo_lance) return FALSE;

    double marge_gauche = 160, marge_droite = 100, marge_haut = 40, marge_bas = 100;
    double zone_x = width - marge_gauche - marge_droite;
    double min_height_per_process = 40;
    double required_height = min_height_per_process * num_processus;
    
    double zone_y = required_height;
    if (zone_y < height - marge_haut - marge_bas) {
        zone_y = height - marge_haut - marge_bas;
    }

    double echelle_x = zone_x / (temps_max + 25);
    
    double hauteur_barre = 30;

    double espacement = zone_y / num_processus;

    double y0 = marge_haut;
    int temps_aff = animation_en_cours ? temps_actuel : temps_max;

    draw_gantt_bars(widget, cr, marge_gauche, echelle_x, hauteur_barre, espacement, y0, temps_aff);

    double y_axe = marge_haut + zone_y + 20;
    draw_gantt_scale(cr, marge_gauche, marge_droite, y_axe, echelle_x, temps_max, width);

    return FALSE;
}

void on_entry_changed(GtkEditable *editable, gpointer user_data) {
    int row = GPOINTER_TO_INT(user_data) / 10;
    int col = GPOINTER_TO_INT(user_data) % 10;
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    
    switch (col) {
        case COL_NOM:
            strncpy(processus_list[row].nom, text, 19);
            processus_list[row].nom[19] = '\0';
            break;
        case COL_ARRIVEE: {
            int val = atoi(text);
            if (val >= 0) processus_list[row].arrivee = val;
            break;
        }
        case COL_DUREE: {
            int val = atoi(text);
            if (val > 0) {
                processus_list[row].duree = val;
                processus_list[row].restant = val;
            }
            break;
        }
        case COL_PRIORITE: {
            int val = atoi(text);
            if (val >= 0) {
                processus_list[row].priorite = val;
                processus_list[row].priorite_dynamique = val;
            }
            break;
        }
    }
}

void on_delete_row_clicked(GtkButton *button G_GNUC_UNUSED, gpointer user_data) {
    int row = GPOINTER_TO_INT(user_data);
    
    if (row < 0 || row >= num_processus) {
        return;
    }

    for (int i = row; i < num_processus - 1; i++) {
        processus_list[i] = processus_list[i + 1];
    }
    num_processus--;
    selected_row = -1;
    
    build_table();
}

gboolean on_row_button_press(GtkWidget *widget, GdkEventButton *event G_GNUC_UNUSED, gpointer user_data) {
    selected_row = GPOINTER_TO_INT(user_data);
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(table_grid));
    for (GList *l = children; l != NULL; l = l->next) {
        GtkWidget *child = GTK_WIDGET(l->data);
        if (child == widget) {
            gtk_widget_set_state_flags(child, GTK_STATE_FLAG_SELECTED, FALSE);
        } else {
            gtk_widget_unset_state_flags(child, GTK_STATE_FLAG_SELECTED);
        }
    }
    g_list_free(children);
    
    return FALSE;
}

void build_table() {
    if (row_widgets) {
        g_list_free_full(row_widgets, (GDestroyNotify)gtk_widget_destroy);
        row_widgets = NULL;
    }
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(table_grid));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);
    
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(header), FALSE);
    
    GtkWidget *delete_spacer = gtk_label_new("");
    gtk_widget_set_size_request(delete_spacer, 40, -1);
    gtk_box_pack_start(GTK_BOX(header), delete_spacer, FALSE, FALSE, 0);
    
    GtkWidget *data_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(data_header), TRUE);
    
    const char *titles[] = {"Nom", "Arrivée", "Durée", "Priorité"};
    const char *colors[] = {"#0ea5e9", "#38bdf8", "#f472b6", "#84cc16"};
    
    for (int col = 0; col < NUM_COLS; col++) {
        GtkWidget *label = gtk_label_new(titles[col]);
        gtk_label_set_xalign(GTK_LABEL(label), 0.5);
        gtk_widget_set_margin_top(label, 8);
        gtk_widget_set_margin_bottom(label, 8);
        
        GtkCssProvider *css = gtk_css_provider_new();
        gchar *css_str = g_strdup_printf("label { font-weight: bold; font-size: 13px; color: %s; }", colors[col]);
        gtk_css_provider_load_from_data(css, css_str, -1, NULL);
        gtk_style_context_add_provider(gtk_widget_get_style_context(label), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_free(css_str);
        g_object_unref(css);
        
        gtk_box_pack_start(GTK_BOX(data_header), label, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(header), data_header, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(table_grid), header);
    gtk_widget_show_all(header);
    
    for (int row = 0; row < num_processus; row++) {
        GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_set_homogeneous(GTK_BOX(row_box), FALSE);
        gtk_widget_set_can_focus(row_box, TRUE);
        gtk_widget_add_events(row_box, GDK_BUTTON_PRESS_MASK);
        g_signal_connect(row_box, "button-press-event", G_CALLBACK(on_row_button_press), GINT_TO_POINTER(row));
        
        GtkWidget *delete_btn = gtk_button_new_with_label("✕");
        gtk_widget_set_size_request(delete_btn, 40, -1);
        g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_row_clicked), GINT_TO_POINTER(row));
        gtk_box_pack_start(GTK_BOX(row_box), delete_btn, FALSE, FALSE, 0);
        
        GtkWidget *data_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_set_homogeneous(GTK_BOX(data_row), TRUE);
        
        gchar buf[16];
        
        GtkWidget *entry_nom = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry_nom), processus_list[row].nom);
        gtk_entry_set_alignment(GTK_ENTRY(entry_nom), 0.5);
        gtk_widget_set_margin_top(entry_nom, 4);
        gtk_widget_set_margin_bottom(entry_nom, 4);
        gtk_widget_set_margin_start(entry_nom, 4);
        gtk_widget_set_margin_end(entry_nom, 4);
        g_signal_connect(entry_nom, "changed", G_CALLBACK(on_entry_changed), GINT_TO_POINTER(row * 10 + COL_NOM));
        gtk_box_pack_start(GTK_BOX(data_row), entry_nom, TRUE, TRUE, 0);
        
        GtkWidget *entry_arrivee = gtk_entry_new();
        snprintf(buf, sizeof(buf), "%d", processus_list[row].arrivee);
        gtk_entry_set_text(GTK_ENTRY(entry_arrivee), buf);
        gtk_entry_set_alignment(GTK_ENTRY(entry_arrivee), 0.5);
        gtk_widget_set_margin_top(entry_arrivee, 4);
        gtk_widget_set_margin_bottom(entry_arrivee, 4);
        gtk_widget_set_margin_start(entry_arrivee, 4);
        gtk_widget_set_margin_end(entry_arrivee, 4);
        g_signal_connect(entry_arrivee, "changed", G_CALLBACK(on_entry_changed), GINT_TO_POINTER(row * 10 + COL_ARRIVEE));
        gtk_box_pack_start(GTK_BOX(data_row), entry_arrivee, TRUE, TRUE, 0);
        
        GtkWidget *entry_duree = gtk_entry_new();
        snprintf(buf, sizeof(buf), "%d", processus_list[row].duree);
        gtk_entry_set_text(GTK_ENTRY(entry_duree), buf);
        gtk_entry_set_alignment(GTK_ENTRY(entry_duree), 0.5);
        gtk_widget_set_margin_top(entry_duree, 4);
        gtk_widget_set_margin_bottom(entry_duree, 4);
        gtk_widget_set_margin_start(entry_duree, 4);
        gtk_widget_set_margin_end(entry_duree, 4);
        g_signal_connect(entry_duree, "changed", G_CALLBACK(on_entry_changed), GINT_TO_POINTER(row * 10 + COL_DUREE));
        gtk_box_pack_start(GTK_BOX(data_row), entry_duree, TRUE, TRUE, 0);
        
        GtkWidget *entry_priorite = gtk_entry_new();
        snprintf(buf, sizeof(buf), "%d", processus_list[row].priorite);
        gtk_entry_set_text(GTK_ENTRY(entry_priorite), buf);
        gtk_entry_set_alignment(GTK_ENTRY(entry_priorite), 0.5);
        gtk_widget_set_margin_top(entry_priorite, 4);
        gtk_widget_set_margin_bottom(entry_priorite, 4);
        gtk_widget_set_margin_start(entry_priorite, 4);
        gtk_widget_set_margin_end(entry_priorite, 4);
        g_signal_connect(entry_priorite, "changed", G_CALLBACK(on_entry_changed), GINT_TO_POINTER(row * 10 + COL_PRIORITE));
        gtk_box_pack_start(GTK_BOX(data_row), entry_priorite, TRUE, TRUE, 0);
        
        gtk_box_pack_start(GTK_BOX(row_box), data_row, TRUE, TRUE, 0);
        gtk_widget_set_margin_start(row_box, 0);
        gtk_widget_set_margin_end(row_box, 0);
        gtk_widget_set_margin_top(row_box, 2);
        gtk_widget_set_margin_bottom(row_box, 2);
        gtk_container_add(GTK_CONTAINER(table_grid), row_box);
        row_widgets = g_list_append(row_widgets, row_box);
    }
    
    gtk_widget_show_all(table_grid);
}

void on_quantum_changed(GtkSpinButton *spin_button, gpointer user_data G_GNUC_UNUSED) {
    global_quantum = gtk_spin_button_get_value_as_int(spin_button);
}

void update_gantt_size() {
    if (!gantt_drawing_area) return;
    
    double min_height_per_process = 40;
    double required_height = min_height_per_process * num_processus + 140;
    
    gtk_widget_set_size_request(gantt_drawing_area, -1, (int)required_height);
}

void on_algorithm_changed(GtkComboBox *widget G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    const gchar *vrai_nom = gtk_combo_box_get_active_id(GTK_COMBO_BOX(algorithm_combo));
    if (!vrai_nom) return;

    char lib_path[256];
    snprintf(lib_path, sizeof(lib_path), "build/politiques/%s", vrai_nom);
    void *handle = dlopen(lib_path, RTLD_LAZY);
    gtk_widget_set_visible(quantum_spin_button, handle && dlsym(handle, "definir_quantum"));
    if (handle) dlclose(handle);
}

void run_scheduler_callback(GtkButton *button G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    const gchar *vrai_nom = gtk_combo_box_get_active_id(GTK_COMBO_BOX(algorithm_combo));
    if (!vrai_nom) return;

    char lib_path[256];
    snprintf(lib_path, sizeof(lib_path), "build/politiques/%s", vrai_nom);

    void *handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        g_printerr("Erreur chargement %s\n", vrai_nom);
        return;
    }

    void (*ordonnancer)(Processus[], int) = dlsym(handle, "ordonnancer");
    void (*definir_quantum)(int) = dlsym(handle, "definir_quantum");

    for (int i = 0; i < num_processus; i++) {
        processus_list[i].restant = processus_list[i].duree;
        processus_list[i].nb_segments = 0;
        processus_list[i].temps_sortie = -1;
        processus_list[i].priorite = processus_list[i].priorite_dynamique;
    }

    if (definir_quantum) definir_quantum(global_quantum);
    if (ordonnancer) ordonnancer(processus_list, num_processus);

    temps_max = 0;
    for (int i = 0; i < num_processus; i++) {
        if (processus_list[i].temps_sortie > temps_max)
            temps_max = processus_list[i].temps_sortie;
    }

    algo_lance = TRUE;
    animation_en_cours = TRUE;
    temps_actuel = 0;
    update_gantt_size();
    g_timeout_add(200, animer, NULL);
    gtk_widget_queue_draw(gantt_drawing_area);

    dlclose(handle);
}

void load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        g_printerr("Impossible d'ouvrir %s\n", filename);
        return;
    }

    num_processus = 0;
    selected_row = -1;

    char ligne[256];
    while (fgets(ligne, sizeof(ligne), f)) {
        if (ligne[0] == '#' || ligne[0] == '\n') continue;

        Processus *p = &processus_list[num_processus];
        if (sscanf(ligne, "%s %d %d %d", p->nom, &p->arrivee, &p->duree, &p->priorite) == 4) {
            p->restant = p->duree;
            p->nb_segments = 0;
            p->temps_sortie = -1;
            p->priorite_dynamique = p->priorite;
            num_processus++;
        }
    }
    fclose(f);
    build_table();
    algo_lance = FALSE;
    gtk_widget_queue_draw(gantt_drawing_area);
}

void open_file_callback(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Ouvrir fichier processus",
                                                    GTK_WINDOW(data),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "Annuler", GTK_RESPONSE_CANCEL,
                                                    "Ouvrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        load_file(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void add_process_callback(GtkButton *button G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    if (num_processus >= 100) {
        g_printerr("Limite: 100 processus maximum\n");
        return;
    }

    Processus *p = &processus_list[num_processus];
    snprintf(p->nom, sizeof(p->nom), "P%d", num_processus + 1);
    p->arrivee = 0;
    p->duree = 10;
    p->priorite = 0;
    p->restant = p->duree;
    p->nb_segments = 0;
    p->temps_sortie = -1;
    p->priorite_dynamique = p->priorite;
    num_processus++;
    
    build_table();
    gtk_adjustment_set_value(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(table_scroll)), 
                            gtk_adjustment_get_upper(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(table_scroll))));
}

void remove_process_callback(GtkButton *button G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    if (selected_row < 0 || selected_row >= num_processus) {
        g_printerr("Veuillez sélectionner un processus à supprimer\n");
        return;
    }

    for (int i = selected_row; i < num_processus - 1; i++) {
        processus_list[i] = processus_list[i + 1];
    }
    num_processus--;
    selected_row = -1;
    
    build_table();
}

static char* get_display_name(const char *filename) {
    static char display_name[100];
    char temp[100];
    strncpy(temp, filename, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    
    char *dot = strrchr(temp, '.');
    if (dot) *dot = '\0';
    
    char *underscore;
    while ((underscore = strchr(temp, '_')) != NULL) {
        *underscore = ' ';
    }
    
    if (temp[0] >= 'a' && temp[0] <= 'z') {
        temp[0] = temp[0] - 'a' + 'A';
    }
    
    strcpy(display_name, temp);
    return display_name;
}

void populate_algorithms() {
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(algorithm_combo));

    DIR *dir = opendir("build/politiques");
    if (!dir) {
        g_printerr("Erreur: impossible d'ouvrir build/politiques\n");
        return;
    }

    struct dirent *entry;
    int current_index = 0;
    int fifo_index = -1;
    
    char **filenames = malloc(100 * sizeof(char*));
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".so") && entry->d_name[0] != '.') {
            filenames[count] = strdup(entry->d_name);
            count++;
        }
    }
    closedir(dir);

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(filenames[i], filenames[j]) > 0) {
                char *temp = filenames[i];
                filenames[i] = filenames[j];
                filenames[j] = temp;
            }
        }
    }

    for (int i = 0; i < count; i++) {
        char *display = get_display_name(filenames[i]);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algorithm_combo), filenames[i], display);
        
        if (strcmp(filenames[i], "fifo.so") == 0) {
            fifo_index = current_index;
        }
        current_index++;
    }

    if (fifo_index >= 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), fifo_index);
    } else if (count > 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 0);
    }

    for (int i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

void launch_gui(int argc, char *argv[], const char* filename) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Ordonnanceur de Processus");
    gtk_window_set_default_size(GTK_WINDOW(window), 1600, 900);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkCssProvider *global_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(global_css,
        "window { font-family: 'Segoe UI', sans-serif; background: #f0f4f8; }"
        "button { padding: 12px 24px; font-size: 14px; font-weight: bold; border-radius: 18px; margin: 6px; background: #B0C4DE; color: #2d3748; border: none; box-shadow: 0 2px 8px rgba(176,196,222,0.3); }"
        "button:hover { background: #9fb5d6; transform: translateY(-1px); box-shadow: 0 4px 12px rgba(176,196,222,0.4); }"
        "combobox, spinbutton { font-size: 13px; padding: 8px; border-radius: 14px; background: white; border: 1px solid #B0C4DE; }"
        "#table_grid { background: white; border-radius: 8px; margin: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); }"
        "#table_grid > box { border-bottom: 1px solid #e9ecef; background: white; }"
        "#table_grid > box:hover { background: #f8f9fa; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(global_css),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(top_bar, 8);
    gtk_widget_set_margin_end(top_bar, 8);
    gtk_widget_set_margin_top(top_bar, 8);
    gtk_widget_set_margin_bottom(top_bar, 6);
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
    gtk_widget_set_no_show_all(quantum_spin_button, TRUE);
    gtk_widget_set_visible(quantum_spin_button, FALSE);

    GtkWidget *run_btn = gtk_button_new_with_label("Lancer l'algorithme");
    g_signal_connect(run_btn, "clicked", G_CALLBACK(run_scheduler_callback), NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), run_btn, FALSE, FALSE, 0);

    gantt_drawing_area = gtk_drawing_area_new();
    g_signal_connect(gantt_drawing_area, "draw", G_CALLBACK(draw_gantt_callback), NULL);
    gtk_widget_set_margin_top(gantt_drawing_area, 4);
    gtk_widget_set_margin_bottom(gantt_drawing_area, 4);
    gtk_widget_set_margin_start(gantt_drawing_area, 4);
    gtk_widget_set_margin_end(gantt_drawing_area, 4);
    
    GtkWidget *gantt_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gantt_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(gantt_scroll), gantt_drawing_area);
    gtk_box_pack_start(GTK_BOX(main_box), gantt_scroll, TRUE, TRUE, 0);

    GtkWidget *table_label = gtk_label_new("Processus");
    gtk_widget_set_margin_start(table_label, 10);
    gtk_widget_set_margin_top(table_label, 8);
    gtk_box_pack_start(GTK_BOX(main_box), table_label, FALSE, FALSE, 0);

    table_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(table_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(table_scroll, -1, 280);
    gtk_widget_set_margin_start(table_scroll, 8);
    gtk_widget_set_margin_end(table_scroll, 8);
    
    table_grid = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(table_grid, "table_grid");
    gtk_container_add(GTK_CONTAINER(table_scroll), table_grid);
    gtk_box_pack_start(GTK_BOX(main_box), table_scroll, FALSE, FALSE, 0);

    GtkWidget *add_btn = gtk_button_new_with_label("+");
    gtk_widget_set_margin_start(add_btn, 8);
    gtk_widget_set_margin_end(add_btn, 8);
    gtk_widget_set_margin_top(add_btn, 4);
    gtk_widget_set_margin_bottom(add_btn, 8);
    g_signal_connect(add_btn, "clicked", G_CALLBACK(add_process_callback), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), add_btn, FALSE, TRUE, 0);

    if (filename) load_file(filename);

    gtk_widget_show_all(window);
    gtk_main();
}
