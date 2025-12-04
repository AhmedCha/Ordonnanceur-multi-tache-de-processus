#include <gtk/gtk.h>
#include <cairo.h>
#include "processus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

// Enums for the columns in the GtkTreeView
enum {
    COL_NOM,
    COL_ARRIVEE,
    COL_DUREE,
    COL_PRIORITE,
    NUM_COLS
};

static GtkListStore *store;
static Processus processus_list[100];
static int num_processus = 0;
static GtkWidget *gantt_drawing_area;
static GtkWidget *results_text_view;
static GtkWidget *algorithm_combo;
static GtkWidget *quantum_spin_button;
int global_quantum = 4; // Default quantum

// Forward declaration
void run_scheduler_callback(GtkButton *button, gpointer user_data);
gboolean draw_gantt_callback(GtkWidget *widget, cairo_t *cr, gpointer data);
void on_algorithm_changed(GtkComboBox *widget, gpointer user_data);
void on_quantum_changed(GtkSpinButton *spin_button, gpointer user_data);


// Callback function for when a cell is edited
void cell_edited_callback(GtkCellRendererText *cell G_GNUC_UNUSED,
                        const gchar *path_string,
                        const gchar *new_text,
                        gpointer data) {
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    int row = gtk_tree_path_get_indices(path)[0];
    int col = GPOINTER_TO_INT(data);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);


    switch (col) {
        case COL_NOM:
            strncpy(processus_list[row].nom, new_text, 19);
            processus_list[row].nom[19] = '\0';
            gtk_list_store_set(store, &iter, col, processus_list[row].nom, -1);
            break;
        case COL_ARRIVEE:
            processus_list[row].arrivee = atoi(new_text);
            gtk_list_store_set(store, &iter, col, processus_list[row].arrivee, -1);
            break;
        case COL_DUREE:
            processus_list[row].duree = atoi(new_text);
            gtk_list_store_set(store, &iter, col, processus_list[row].duree, -1);
            break;
        case COL_PRIORITE:
            processus_list[row].priorite = atoi(new_text);
             gtk_list_store_set(store, &iter, col, processus_list[row].priorite, -1);
            break;
    }

    gtk_tree_path_free(path);
}


// Function to populate the GtkTreeView with process data
void populate_tree_view() {
    gtk_list_store_clear(store);
    GtkTreeIter iter;

    for (int i = 0; i < num_processus; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COL_NOM, processus_list[i].nom,
                           COL_ARRIVEE, processus_list[i].arrivee,
                           COL_DUREE, processus_list[i].duree,
                           COL_PRIORITE, processus_list[i].priorite,
                           -1);
    }
}


// Callback function for opening a file
void open_file_callback(GtkButton *button G_GNUC_UNUSED, gpointer user_data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(user_data),
                                         action,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        FILE *file = fopen(filename, "r");
        if (file) {
            char line[256];
            num_processus = 0;
            while (fgets(line, sizeof(line), file) && num_processus < 100) {
                if (line[0] != '#' && strspn(line, " \t\r\n") != strlen(line)) {
                    if (sscanf(line, "%s %d %d %d",
                               processus_list[num_processus].nom,
                               &processus_list[num_processus].arrivee,
                               &processus_list[num_processus].duree,
                               &processus_list[num_processus].priorite) == 4) {
                        num_processus++;
                    }
                }
            }
            fclose(file);
            populate_tree_view();
        }
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

// Function to add a column to the GtkTreeView
void add_column(GtkTreeView *treeview, const char *title, int col_num) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", (GCallback)cell_edited_callback, GINT_TO_POINTER(col_num));

    GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(title,
                                                                      renderer,
                                                                      "text", col_num,
                                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
}

// Populate algorithm selector
void populate_algorithms() {
    DIR *dir = opendir("build/politiques");
    if (!dir) {
        perror("Could not open build/politiques directory");
        return;
    }
    
    char *fifo_id = NULL;
    int fifo_index = -1;
    int current_index = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".so")) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), entry->d_name);
            if(strcmp(entry->d_name, "fifo.so") == 0) {
                fifo_index = current_index;
            }
            current_index++;
        }
    }
    closedir(dir);

    if(fifo_index != -1) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), fifo_index);
    }
}


// Function to create and show the main GUI window
void launch_gui(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Process Scheduler");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *open_button = gtk_button_new_with_label("Open Process File");
    g_signal_connect(open_button, "clicked", G_CALLBACK(open_file_callback), window);
    gtk_box_pack_start(GTK_BOX(vbox), open_button, FALSE, FALSE, 0);

    GtkWidget *treeview = gtk_tree_view_new();
    add_column(GTK_TREE_VIEW(treeview), "Name", COL_NOM);
    add_column(GTK_TREE_VIEW(treeview), "Arrival", COL_ARRIVEE);
    add_column(GTK_TREE_VIEW(treeview), "Duration", COL_DUREE);
    add_column(GTK_TREE_VIEW(treeview), "Priority", COL_PRIORITE);

    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store); 

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    
    // Algorithm selector and run button
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    algorithm_combo = gtk_combo_box_text_new();
    populate_algorithms();
    g_signal_connect(algorithm_combo, "changed", G_CALLBACK(on_algorithm_changed), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), algorithm_combo, TRUE, TRUE, 0);

    quantum_spin_button = gtk_spin_button_new_with_range(1, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(quantum_spin_button), global_quantum);
    g_signal_connect(quantum_spin_button, "value-changed", G_CALLBACK(on_quantum_changed), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), quantum_spin_button, FALSE, FALSE, 0);


    GtkWidget *run_button = gtk_button_new_with_label("Run Scheduler");
    g_signal_connect(run_button, "clicked", G_CALLBACK(run_scheduler_callback), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), run_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    // Gantt chart drawing area
    gantt_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(gantt_drawing_area, -1, 100);
    g_signal_connect(gantt_drawing_area, "draw", G_CALLBACK(draw_gantt_callback), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), gantt_drawing_area, FALSE, FALSE, 0);

    // Results text view
    results_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(results_text_view), FALSE);
    GtkWidget *results_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(results_scrolled_window), results_text_view);
    gtk_box_pack_start(GTK_BOX(vbox), results_scrolled_window, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    on_algorithm_changed(GTK_COMBO_BOX(algorithm_combo), NULL);
    gtk_main();
}

void on_quantum_changed(GtkSpinButton *spin_button, gpointer user_data G_GNUC_UNUSED) {
    global_quantum = gtk_spin_button_get_value_as_int(spin_button);
}

void on_algorithm_changed(GtkComboBox *widget, gpointer user_data G_GNUC_UNUSED) {
    gchar *active_algo = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    if (active_algo != NULL) {
        if (strcmp(active_algo, "round_robin.so") == 0) {
            gtk_widget_show(quantum_spin_button);
        } else {
            gtk_widget_hide(quantum_spin_button);
        }
        g_free(active_algo);
    }
}


void run_scheduler_callback(GtkButton *button G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    gchar *active_lib = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(algorithm_combo));
    if (!active_lib) return;

    // The global_quantum is now updated by the on_quantum_changed callback

    char lib_path[256];
    snprintf(lib_path, sizeof(lib_path), "build/politiques/%s", active_lib);
    g_free(active_lib);

    void *lib_handle = dlopen(lib_path, RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Error loading %s: %s\n", lib_path, dlerror());
        return;
    }

    void (*ordonnancer)(Processus[], int);
    *(void **)(&ordonnancer) = dlsym(lib_handle, "ordonnancer");

    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Error finding symbol: %s\n", error);
        dlclose(lib_handle);
        return;
    }
    
    // Reset data
    for(int i = 0; i < num_processus; i++) {
        processus_list[i].restant = processus_list[i].duree;
        processus_list[i].nb_segments = 0;
    }

    printf("Running scheduler: %s\n", lib_path);
    fflush(stdout);

    ordonnancer(processus_list, num_processus);

    printf("Scheduler finished.\n");
    fflush(stdout);

    dlclose(lib_handle);

    // Update results and Gantt chart
    gtk_widget_queue_draw(gantt_drawing_area);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(results_text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
    char line[256];
    
    for (int i = 0; i < num_processus; i++) {
        sprintf(line, "Process %s: Arrival=%d, Exit=%d\n", processus_list[i].nom, processus_list[i].arrivee, processus_list[i].temps_sortie);
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, line, -1);
    }
}

// Draw the Gantt chart
gboolean draw_gantt_callback(GtkWidget *widget, cairo_t *cr, gpointer data G_GNUC_UNUSED) {
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    int max_time = 0;
    for (int i = 0; i < num_processus; i++) {
        if (processus_list[i].temps_sortie > max_time) {
            max_time = processus_list[i].temps_sortie;
        }
    }

    if (max_time == 0) return FALSE;

    double time_scale = (double)width / max_time;
    double bar_height = height / (num_processus + 1);

    for (int i = 0; i < num_processus; i++) {
        for (int s = 0; s < processus_list[i].nb_segments; s++) {
            double x = processus_list[i].diagramme_gantt[s].debut * time_scale;
            double y = (i + 0.5) * bar_height;
            double w = (processus_list[i].diagramme_gantt[s].fin - processus_list[i].diagramme_gantt[s].debut) * time_scale;
            
            cairo_set_source_rgb(cr, (i % 7) / 6.0, (i * 3 % 7) / 6.0, (i * 5 % 7) / 6.0); // Simple color variation
            cairo_rectangle(cr, x, y, w, bar_height * 0.8);
            cairo_fill(cr);
            
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_move_to(cr, x + 2, y + bar_height * 0.5);
            cairo_show_text(cr, processus_list[i].nom);
        }
    }
    return FALSE;
}