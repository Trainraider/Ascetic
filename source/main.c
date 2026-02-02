#include "data.h"
#include "app_window.h"
#include "uri.h"
#include "version.h"
#include "webview.h"
#include <adwaita.h>
#include <gtk/gtk.h>
#include "globals.h"
#include <webkit/webkit.h>

AdwApplication* app          = NULL;
GIcon*          new_tab_icon = NULL;
GHashTable*     app_windows  = NULL;
AsceticAppWindow* active_window = NULL;

void on_destroy_window(AsceticAppWindow* window, gpointer user_data)
{
        (void)user_data;
        g_hash_table_remove(app_windows, window);
        if (active_window == window) {
                GHashTableIter iter;
                gpointer       key, value;
                g_hash_table_iter_init(&iter, app_windows);
                if (g_hash_table_iter_next(&iter, &key, &value)) {
                        active_window = ASCETIC_APP_WINDOW(key);
                } else {
                        active_window = NULL;
                }
        }
}


void on_window_became_active(AsceticAppWindow* self, gpointer user_data)
{
        (void)self;
        (void)user_data;
        active_window = self;
}

void on_window_creates_window(AsceticAppWindow* self, AsceticAppWindow* new_window, gpointer user_data)
{
        (void)self;
        (void)user_data;
        g_signal_connect(new_window, "became-active", G_CALLBACK(on_window_became_active), NULL);
        g_signal_connect(new_window, "window-created", G_CALLBACK(on_window_creates_window), NULL);
        g_signal_connect(new_window, "destroy", G_CALLBACK(on_destroy_window), NULL);
        g_hash_table_add(app_windows, new_window);
}

AsceticAppWindow* create_window(void)
{
        AsceticAppWindow* window = ascetic_app_window_new();
        g_signal_connect(window, "became-active", G_CALLBACK(on_window_became_active), NULL);
        g_signal_connect(window, "window-created", G_CALLBACK(on_window_creates_window), NULL);
        g_signal_connect(window, "destroy", G_CALLBACK(on_destroy_window), NULL);
        g_hash_table_add(app_windows, window);
        return window;
}

void
on_open(GtkApplication* self, GFile** files, gint n_files, gchar* hint, gpointer user_data)
{
        (void)user_data;
        (void)hint;
        AsceticAppWindow* window;
        if (active_window) {
                window = active_window;
        } else {
                window = create_window();
                gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(self));
                gtk_window_present(GTK_WINDOW(window));
        }
        active_window = window;
        for (gint i = 0; i < n_files; i++) {
                GFile*            file    = files[i];
                AdwTabPage*       tab     = new_tab(GTK_WIDGET(window), window);
                WebKitWebView*    webview = tab_get_webview(tab);
                gchar*            uri     = g_file_get_uri(file);
                webkit_web_view_load_uri(webview, uri);
                g_free(uri);
        }
}

void activate(GtkApplication* self, gpointer user_data)
{
        (void)user_data;
        AsceticAppWindow* window  = create_window();
        active_window = window;
        AdwTabPage*       tab     = new_tab(GTK_WIDGET(window), window);
        WebKitWebView*    webview = tab_get_webview(tab);
        webkit_web_view_load_uri(webview, "https://search.brave.com");

        gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
        gtk_window_present(GTK_WINDOW(window));
}

void startup(GtkApplication* self, gpointer user_data)
{
        (void)user_data;
        uri_init();
        browser_session_init();
        new_tab_icon = g_themed_icon_new("applications-internet-symbolic");
        app_windows  = g_hash_table_new(g_direct_hash, g_direct_equal);
}

void shutdown(GtkApplication* self, gpointer user_data)
{
        (void)self;
        (void)user_data;
        g_hash_table_destroy(app_windows);
        g_object_unref(new_tab_icon);
        browser_session_cleanup();
}

int main(int argc, char* argv[])
{
        if (argc >= 2) {
                if (0 == strcmp(argv[1], "--version"))
                        version();
        }

        app          = adw_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
        g_signal_connect(app, "startup", G_CALLBACK(startup), NULL);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        g_signal_connect(app, "open", G_CALLBACK(on_open), NULL);
        g_signal_connect(app, "shutdown", G_CALLBACK(shutdown), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);
        g_object_unref(app);

        return status;
}