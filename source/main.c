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

void
on_open(GtkApplication* self, GFile** files, gint n_files, gchar* hint, gpointer user_data)
{
        (void)user_data;
        (void)hint;
        AsceticAppWindow* window  = ascetic_app_window_new();
        for (gint i = 0; i < n_files; i++) {
                GFile*            file    = files[i];
                AdwTabPage*       tab     = new_tab(GTK_WIDGET(window), window);
                WebKitWebView*    webview = tab_get_webview(tab);
                gchar*            uri     = g_file_get_uri(file);
                webkit_web_view_load_uri(webview, uri);
                g_free(uri);
        }
        gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(self));
        gtk_window_present(GTK_WINDOW(window));
}

void activate(GtkApplication* app, gpointer user_data)
{
        (void)user_data;
        AsceticAppWindow* window  = ascetic_app_window_new();
        AdwTabPage*       tab     = new_tab(GTK_WIDGET(window), window);
        WebKitWebView*    webview = tab_get_webview(tab);
        webkit_web_view_load_uri(webview, "https://search.brave.com");

        gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
        gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char* argv[])
{
        if (argc >= 2) {
                if (0 == strcmp(argv[1], "--version"))
                        version();
        }

        uri_init();
        browser_session_init();
        new_tab_icon = g_themed_icon_new("applications-internet-symbolic");
        app          = adw_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        g_signal_connect(app, "open", G_CALLBACK(on_open), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);

        g_object_unref(app);
        g_object_unref(new_tab_icon);
        browser_session_cleanup();
        uri_cleanup();

        return status;
}