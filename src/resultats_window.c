#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "processus.h"

static GtkWidget *resultats_window = NULL;

static void on_resultats_window_destroy(GtkWidget *widget, gpointer data) {
    resultats_window = NULL;
}

void afficher_fenetre_resultats(Processus processus_list[], int num_processus) {
    if (resultats_window) {
        gtk_widget_destroy(resultats_window);
        resultats_window = NULL;
    }

    resultats_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(resultats_window), "Résultats de l'Ordonnancement");
    gtk_window_set_default_size(GTK_WINDOW(resultats_window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(resultats_window), GTK_WIN_POS_CENTER);
    g_signal_connect(resultats_window, "destroy", G_CALLBACK(on_resultats_window_destroy), NULL);

    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "window { background: #f5f7fa; font-family: 'Segoe UI', sans-serif; }"
        "#header_box { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); padding: 20px; }"
        "#title_label { color: white; font-size: 22px; font-weight: bold; }"
        "#subtitle_label { color: rgba(255,255,255,0.9); font-size: 13px; margin-top: 4px; }"
        "#table_header { background: #4a5568; padding: 12px; border-radius: 8px 8px 0 0; }"
        "#table_header label { color: white; font-weight: bold; font-size: 14px; }"
        "#table_row { background: white; padding: 10px; border-bottom: 1px solid #e2e8f0; }"
        "#table_row:hover { background: #f7fafc; }"
        "#table_row label { font-size: 13px; color: #2d3748; }"
        "#stats_box { background: white; border-radius: 8px; padding: 16px; margin: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }"
        "#stats_label { font-size: 13px; color: #4a5568; margin: 4px; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(resultats_window), main_vbox);

    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_name(header_box, "header_box");
    
    GtkWidget *title_label = gtk_label_new("Résultat de l'Ordonnancement");
    gtk_widget_set_name(title_label, "title_label");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.5);
    gtk_box_pack_start(GTK_BOX(header_box), title_label, FALSE, FALSE, 0);
    
    
    gtk_box_pack_start(GTK_BOX(main_vbox), header_box, FALSE, FALSE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), 
                                   GTK_POLICY_AUTOMATIC, 
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_margin_start(scrolled_window, 12);
    gtk_widget_set_margin_end(scrolled_window, 12);
    gtk_widget_set_margin_top(scrolled_window, 12);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *table_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(scrolled_window), table_vbox);

    GtkWidget *header_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(header_row, "table_header");
    gtk_box_set_homogeneous(GTK_BOX(header_row), TRUE);
    
    const char *headers[] = {"Processus", "Arrivée", "Sortie"};
    for (int i = 0; i < 3; i++) {
        GtkWidget *header_label = gtk_label_new(headers[i]);
        gtk_label_set_xalign(GTK_LABEL(header_label), 0.5);
        gtk_box_pack_start(GTK_BOX(header_row), header_label, TRUE, TRUE, 8);
    }
    gtk_box_pack_start(GTK_BOX(table_vbox), header_row, FALSE, FALSE, 0);

    int temps_total = 0;
    for (int i = 0; i < num_processus; i++) {
        GtkWidget *data_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_name(data_row, "table_row");
        gtk_box_set_homogeneous(GTK_BOX(data_row), TRUE);
        
        GtkWidget *nom_label = gtk_label_new(processus_list[i].nom);
        gtk_label_set_xalign(GTK_LABEL(nom_label), 0.5);
        gtk_box_pack_start(GTK_BOX(data_row), nom_label, TRUE, TRUE, 8);
        
        char arrivee_str[32];
        snprintf(arrivee_str, sizeof(arrivee_str), "%d", processus_list[i].arrivee);
        GtkWidget *arrivee_label = gtk_label_new(arrivee_str);
        gtk_label_set_xalign(GTK_LABEL(arrivee_label), 0.5);
        gtk_box_pack_start(GTK_BOX(data_row), arrivee_label, TRUE, TRUE, 8);
        
        char sortie_str[32];
        snprintf(sortie_str, sizeof(sortie_str), "%d", processus_list[i].temps_sortie);
        GtkWidget *sortie_label = gtk_label_new(sortie_str);
        gtk_label_set_xalign(GTK_LABEL(sortie_label), 0.5);
        gtk_box_pack_start(GTK_BOX(data_row), sortie_label, TRUE, TRUE, 8);
        
        gtk_box_pack_start(GTK_BOX(table_vbox), data_row, FALSE, FALSE, 0);
        
        if (processus_list[i].temps_sortie > temps_total) {
            temps_total = processus_list[i].temps_sortie;
        }
    }

    double temps_rotation_total = 0.0;
    double temps_attente_total = 0.0;
    
    for (int i = 0; i < num_processus; i++) {
        int temps_rotation = processus_list[i].temps_sortie - processus_list[i].arrivee;
        temps_rotation_total += temps_rotation;
        
        int temps_attente = temps_rotation - processus_list[i].duree;
        temps_attente_total += temps_attente;
    }
    
    double temps_rotation_moyen = (num_processus > 0) ? (temps_rotation_total / num_processus) : 0.0;
    double temps_attente_moyen = (num_processus > 0) ? (temps_attente_total / num_processus) : 0.0;

    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_name(stats_box, "stats_box");
    
    GtkWidget *stats_title = gtk_label_new("Statistiques");
    gtk_widget_set_name(stats_title, "subtitle_label");
    gtk_label_set_xalign(GTK_LABEL(stats_title), 0.0);
    gtk_widget_set_margin_bottom(stats_title, 8);
    
    GtkCssProvider *stats_title_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(stats_title_css,
        "label { color: #2d3748; font-size: 16px; font-weight: bold; }",
        -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(stats_title),
                                   GTK_STYLE_PROVIDER(stats_title_css),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_box_pack_start(GTK_BOX(stats_box), stats_title, FALSE, FALSE, 0);
    
    char nb_processus_str[128];
    snprintf(nb_processus_str, sizeof(nb_processus_str), "Nombre de processus: %d", num_processus);
    GtkWidget *nb_processus_label = gtk_label_new(nb_processus_str);
    gtk_widget_set_name(nb_processus_label, "stats_label");
    gtk_label_set_xalign(GTK_LABEL(nb_processus_label), 0.0);
    gtk_box_pack_start(GTK_BOX(stats_box), nb_processus_label, FALSE, FALSE, 0);
    
    char temps_total_str[128];
    snprintf(temps_total_str, sizeof(temps_total_str), "Temps total d'exécution: %d seconde(s)", temps_total);
    GtkWidget *temps_total_label = gtk_label_new(temps_total_str);
    gtk_widget_set_name(temps_total_label, "stats_label");
    gtk_label_set_xalign(GTK_LABEL(temps_total_label), 0.0);
    gtk_box_pack_start(GTK_BOX(stats_box), temps_total_label, FALSE, FALSE, 0);
    
    char temps_rotation_str[128];
    snprintf(temps_rotation_str, sizeof(temps_rotation_str), "Temps de rotation moyen: %.2f seconde(s)", temps_rotation_moyen);
    GtkWidget *temps_rotation_label = gtk_label_new(temps_rotation_str);
    gtk_widget_set_name(temps_rotation_label, "stats_label");
    gtk_label_set_xalign(GTK_LABEL(temps_rotation_label), 0.0);
    gtk_box_pack_start(GTK_BOX(stats_box), temps_rotation_label, FALSE, FALSE, 0);
    
    char temps_attente_str[128];
    snprintf(temps_attente_str, sizeof(temps_attente_str), "Temps d'attente moyen: %.2f seconde(s)", temps_attente_moyen);
    GtkWidget *temps_attente_label = gtk_label_new(temps_attente_str);
    gtk_widget_set_name(temps_attente_label, "stats_label");
    gtk_label_set_xalign(GTK_LABEL(temps_attente_label), 0.0);
    gtk_box_pack_start(GTK_BOX(stats_box), temps_attente_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_vbox), stats_box, FALSE, FALSE, 0);

    GtkWidget *close_button = gtk_button_new_with_label("Fermer");
    gtk_widget_set_margin_start(close_button, 12);
    gtk_widget_set_margin_end(close_button, 12);
    gtk_widget_set_margin_bottom(close_button, 12);
    gtk_widget_set_margin_top(close_button, 8);
    g_signal_connect_swapped(close_button, "clicked", 
                             G_CALLBACK(gtk_widget_destroy), 
                             resultats_window);
    gtk_box_pack_start(GTK_BOX(main_vbox), close_button, FALSE, FALSE, 0);

    gtk_widget_show_all(resultats_window);
}