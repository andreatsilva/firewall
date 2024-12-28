#include <gtk-3.0/gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Function to execute shell commands
void run_command(const char *command) {
    int ret = system(command);
    if (ret != 0) {
        g_printerr("Command failed: %s\n", command);
    }
}

// Callback to block a port
void on_block_port_clicked(GtkButton *button, gpointer port_entry) {
    const char *port = gtk_entry_get_text(GTK_ENTRY(port_entry));
    if (strlen(port) > 0) {
        char command[128];
        snprintf(command, sizeof(command), "iptables -A INPUT -p tcp --dport %s -j DROP", port);
        run_command(command);
        g_print("Blocked port %s\n", port);
    } else {
        g_print("Please enter a valid port.\n");
    }
}

// Callback to block an IP
void on_block_ip_clicked(GtkButton *button, gpointer ip_entry) {
    const char *ip = gtk_entry_get_text(GTK_ENTRY(ip_entry));
    if (strlen(ip) > 0) {
        char command[128];
        snprintf(command, sizeof(command), "iptables -A INPUT -s %s -j DROP", ip);
        run_command(command);
        g_print("Blocked IP %s\n", ip);
    } else {
        g_print("Please enter a valid IP address.\n");
    }
}

// Callback to refresh rules
void on_refresh_rules_clicked(GtkButton *button, GtkTextView *rules_view) {
    char buffer[4096];
    FILE *pipe = popen("iptables -L -v -n", "r");
    if (!pipe) {
        g_printerr("Failed to fetch rules.\n");
        return;
    }

    GtkTextBuffer *buffer_text = gtk_text_view_get_buffer(rules_view);
    gtk_text_buffer_set_text(buffer_text, "", -1);

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        gtk_text_buffer_insert_at_cursor(buffer_text, buffer, -1);
    }

    pclose(pipe);
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *grid;
    GtkWidget *port_label, *port_entry, *block_port_button;
    GtkWidget *ip_label, *ip_entry, *block_ip_button;
    GtkWidget *rules_view, *refresh_button;
    GtkTextBuffer *buffer;

    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Firewall Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Port blocking UI
    port_label = gtk_label_new("Port to Block:");
    port_entry = gtk_entry_new();
    block_port_button = gtk_button_new_with_label("Block Port");
    gtk_grid_attach(GTK_GRID(grid), port_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), port_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), block_port_button, 3, 0, 1, 1);

    // IP blocking UI
    ip_label = gtk_label_new("IP to Block:");
    ip_entry = gtk_entry_new();
    block_ip_button = gtk_button_new_with_label("Block IP");
    gtk_grid_attach(GTK_GRID(grid), ip_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ip_entry, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), block_ip_button, 3, 1, 1, 1);

    // Rules display and refresh button
    rules_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(rules_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(rules_view), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(rules_view));
    gtk_text_buffer_set_text(buffer, "Firewall rules will appear here...", -1);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), rules_view);
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 2, 4, 1);

    refresh_button = gtk_button_new_with_label("Refresh Rules");
    gtk_grid_attach(GTK_GRID(grid), refresh_button, 0, 3, 4, 1);

    // Connect button signals
    g_signal_connect(block_port_button, "clicked", G_CALLBACK(on_block_port_clicked), port_entry);
    g_signal_connect(block_ip_button, "clicked", G_CALLBACK(on_block_ip_clicked), ip_entry);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_rules_clicked), rules_view);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
