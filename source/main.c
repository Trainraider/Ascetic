#include "data.h"
#include "app_window.h"
#include "settings_page.h"
#include "uri.h"
#include "version.h"
#include "webview.h"
#include <adwaita.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webkit/webkit.h>
#include "defer.h"
#include "globals.h"

AdwApplication* app          = NULL;
GIcon*          new_tab_icon = NULL;

void check_gobject(GObject* obj, gchar* failure_msg)
{
        if (obj)
                return;
        if (failure_msg) {
                g_printerr("%s", failure_msg);
        } else {
                g_printerr("Error: Failed to get GObject\n");
        }
        g_application_quit(G_APPLICATION(app));
        exit(1);
}

#define BUILDER_GET_OBJECT(builder, type, TYPE, name)                             \
        ({                                                                        \
                type* obj = TYPE(gtk_builder_get_object(builder, name));          \
                check_gobject(G_OBJECT(obj), "Error: Failed to get " name ".\n"); \
                obj;                                                              \
        })

#define ROOT(widget) ASCETIC_APP_WINDOW(gtk_widget_get_root(GTK_WIDGET(widget)))

void show_tab_overview(GtkWidget* widget, gpointer user_data)
{
        (void)user_data;
        adw_tab_overview_set_open(ROOT(widget)->web_page, TRUE);
}

void on_back_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)user_data;
        WebKitWebView* active_web_view = ROOT(widget)->active_web_view;
        if (active_web_view)
                webkit_web_view_go_back(active_web_view);
}

void on_forward_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)user_data;
        WebKitWebView* active_web_view = ROOT(widget)->active_web_view;
        if (active_web_view)
                webkit_web_view_go_forward(active_web_view);
}

void on_refresh_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)user_data;
        WebKitWebView* active_web_view = ROOT(widget)->active_web_view;
        if (active_web_view)
                webkit_web_view_reload(active_web_view);
}

void on_open_settings_button_clicked(GtkWidget* widget, gpointer user_data)
{
        AsceticAppWindow* root          = ROOT(widget);
        GtkStack*         stack         = root->stack_main;
        GtkWidget*        settings_page = GTK_WIDGET(root->settings_page);
        gtk_stack_set_visible_child(stack, settings_page);
        gtk_widget_set_visible(GTK_WIDGET(root->web_tab_bar), FALSE);
}

void on_close_settings_button_clicked(GtkWidget* widget, gpointer user_data)
{
        AsceticAppWindow* root      = ROOT(widget);
        GtkStack*         stack     = root->stack_main;
        GtkWidget*        main_page = GTK_WIDGET(root->web_page);
        gtk_stack_set_visible_child(stack, main_page);
        gtk_widget_set_visible(GTK_WIDGET(root->web_tab_bar), TRUE);
}

void load_url_from_entry(GtkWidget* entry, gpointer user_data)
{
        S_(void)
                user_data;
                AsceticAppWindow* root            = ROOT(entry);
                WebKitWebView*    active_web_view = root->active_web_view;
                if (!active_web_view)
                        return;
                const char* url        = gtk_editable_get_text(GTK_EDITABLE(entry));
                ParsedUri   parsed_uri = uri_parse(url);
                defer(dg_free, parsed_uri.str);
                gtk_editable_set_text(GTK_EDITABLE(entry), parsed_uri.str);
                if (parsed_uri.is_uri) {
                        webkit_web_view_load_uri(active_web_view, parsed_uri.str);
                } else {
                        S_
                                char* search_url = str_to_brave_search_url(parsed_uri.str);
                                defer(dg_free, search_url);
                                webkit_web_view_load_uri(active_web_view, search_url);
                        _S
                }
        _S
}

void on_tab_page_attached(AdwTabView* self, AdwTabPage* page, gint position, gpointer user_data)
{
        (void)user_data;
        adw_tab_view_set_selected_page(self, page);
}

void activate(GtkApplication* app, gpointer user_data)
{
        (void)user_data;
        GtkBuilderScope* scope = gtk_builder_cscope_new();
        gtk_builder_cscope_add_callback(scope, load_url_from_entry);
        gtk_builder_cscope_add_callback(scope, on_back_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_forward_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_refresh_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_open_settings_button_clicked);
        gtk_builder_cscope_add_callback(scope, on_close_settings_button_clicked);
        gtk_builder_cscope_add_callback(scope, show_tab_overview);
        gtk_builder_cscope_add_callback(scope, on_tab_page_attached);
        gtk_builder_cscope_add_callback(scope, new_tab);
        GtkBuilder* builder = gtk_builder_new();
        gtk_builder_set_scope(builder, scope);
        GError* error = NULL;
        gtk_builder_add_from_resource(builder, APP_PREFIX "/window_main.ui", &error);
        if (error) {
                g_printerr("Error: Failed to load UI resource: %s\n", error->message);
                g_clear_error(&error);
                exit(1);
        }

        AsceticAppWindow* window = ASCETIC_APP_WINDOW(gtk_builder_get_object(builder, "main_window"));
        gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
        gtk_window_present(GTK_WINDOW(window));

        g_object_unref(scope);
        g_object_unref(builder);
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