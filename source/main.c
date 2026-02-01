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
        app          = adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);

        g_object_unref(app);
        g_object_unref(new_tab_icon);
        browser_session_cleanup();
        uri_cleanup();

        return status;
}