#include "data.h"
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

GtkBuilder*     builder         = NULL;
AdwApplication* app             = NULL;
AdwTabOverview* tab_overview    = NULL;
AdwTabBar*      tab_bar         = NULL;
AdwTabView*     tab_view        = NULL;
WebKitWebView*  active_web_view = NULL;
GIcon*          new_tab_icon    = NULL;
GtkEntry*       url_entry       = NULL;

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

void show_tab_overview(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        (void)user_data;
        adw_tab_overview_set_open(tab_overview, TRUE);
}

void on_back_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        (void)user_data;
        webkit_web_view_go_back(active_web_view);
}

void on_forward_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        (void)user_data;
        webkit_web_view_go_forward(active_web_view);
}

void on_refresh_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        (void)user_data;
        webkit_web_view_reload(active_web_view);
}

typedef struct
{
        GtkStack*  stack;
        GtkWidget* page;
} StackState;

static StackState open_settings_state;
static StackState back_to_main_state;

void on_open_settings_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        GtkStack*  stack         = open_settings_state.stack;
        GtkWidget* settings_page = open_settings_state.page;
        gtk_stack_set_visible_child(stack, settings_page);
}

void on_close_settings_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        GtkStack*  stack     = back_to_main_state.stack;
        GtkWidget* main_page = back_to_main_state.page;
        gtk_stack_set_visible_child(stack, main_page);
}

void load_url_from_entry(GtkWidget* entry, gpointer user_data)
{
        S_(void)
                user_data;
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

void on_tab_bar_visibility_changed(AdwTabBar* tab_bar, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        GtkWidget* upper_new_tab_button = GTK_WIDGET(user_data);
        gboolean   tabs_revealed        = adw_tab_bar_get_tabs_revealed(tab_bar);
        if (tabs_revealed) {
                // gtk_widget_set_visible(upper_new_tab_button, FALSE);
                gtk_widget_set_opacity(upper_new_tab_button, 0.0);
                gtk_widget_set_sensitive(upper_new_tab_button, FALSE);
        } else {
                // gtk_widget_set_visible(upper_new_tab_button, TRUE);
                gtk_widget_set_opacity(upper_new_tab_button, 1.0);
                gtk_widget_set_sensitive(upper_new_tab_button, TRUE);
        }
}

#define BUILDER_GET_OBJECT(builder, type, TYPE, name)                             \
        ({                                                                        \
                type* obj = TYPE(gtk_builder_get_object(builder, name));          \
                check_gobject(G_OBJECT(obj), "Error: Failed to get " name ".\n"); \
                obj;                                                              \
        })

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
        gtk_builder_cscope_add_callback(scope, new_tab);
        builder = gtk_builder_new();
        gtk_builder_set_scope(builder, scope);
        GError* error = NULL;
        gtk_builder_add_from_resource(builder, APP_PREFIX "/window_main.ui", &error);
        if (error) {
                g_printerr("Error: Failed to load UI resource: %s\n", error->message);
                g_clear_error(&error);
                exit(1);
        }
        g_object_unref(scope);

        AdwWindow*    window     = BUILDER_GET_OBJECT(builder, AdwWindow, ADW_WINDOW, "window_main");
        GtkStack*     stack_main = BUILDER_GET_OBJECT(builder, GtkStack, GTK_STACK, "stack_main");
        GtkStackPage* main_page  = BUILDER_GET_OBJECT(builder, GtkStackPage, GTK_STACK_PAGE, "main_page");

        GtkStackPage* settings_page = BUILDER_GET_OBJECT(builder, GtkStackPage, GTK_STACK_PAGE, "settings_page");
        GtkWidget*    template      = BUILDER_GET_OBJECT(builder, GtkWidget, GTK_WIDGET, "settings_page_template");

        tab_overview                    = BUILDER_GET_OBJECT(builder, AdwTabOverview, ADW_TAB_OVERVIEW, "web_tabs_overview");
        tab_bar                         = BUILDER_GET_OBJECT(builder, AdwTabBar, ADW_TAB_BAR, "web_tab_bar");
        tab_view                        = BUILDER_GET_OBJECT(builder, AdwTabView, ADW_TAB_VIEW, "web_view");
        url_entry                       = BUILDER_GET_OBJECT(builder, GtkEntry, GTK_ENTRY, "url_entry");
        GtkWidget* upper_new_tab_button = BUILDER_GET_OBJECT(builder, GtkWidget, GTK_WIDGET, "upper_new_tab_button");

        GtkWidget* close_settings_button = template_app_settings_page_get_close_settings_button(TEMPLATE_APP_SETTINGS_PAGE(template));
        check_gobject(G_OBJECT(close_settings_button), "Error: Failed to get the close_settings_button.\n");
        GtkWidget* open_settings_button = BUILDER_GET_OBJECT(builder, GtkWidget, GTK_WIDGET, "open_settings_button");

        AdwTabPage*    tab     = new_tab(NULL, NULL);
        WebKitWebView* webview = tab_get_webview(tab);
        webkit_web_view_load_uri(webview, "https://search.brave.com");
        active_web_view = webview;

        g_signal_connect(tab_bar, "notify::tabs-revealed", G_CALLBACK(on_tab_bar_visibility_changed), upper_new_tab_button);
        g_signal_connect(tab_view, "notify::selected-page", G_CALLBACK(on_tab_changed), NULL);

        open_settings_state.stack = stack_main;
        open_settings_state.page  = gtk_stack_page_get_child(settings_page);

        back_to_main_state.stack = stack_main;
        back_to_main_state.page  = gtk_stack_page_get_child(main_page);

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
        new_tab_icon = g_themed_icon_new("xsi-applications-internet-symbolic");
        app          = adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);

        if (builder)
                g_object_unref(builder);
        g_object_unref(app);
        g_object_unref(new_tab_icon);
        browser_session_cleanup();
        uri_cleanup();

        return status;
}