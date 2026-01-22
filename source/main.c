#include "data.h"
#include "defer.h"
#include "settings_page.h"
#include "uri.h"
#include "version.h"
#include <adwaita.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webkit/webkit.h>

static GtkBuilder*     builder;
static AdwApplication* app;
static WebKitWebView*  web_view;

void dg_free(void* ptr)
{
        void** _ptr = (void**)ptr;
        if (*_ptr) {
                g_free(*_ptr);
                *_ptr = NULL;
        }
}

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

typedef struct
{
        GtkLabel* label;
        int       value;
        char      text[32];
} CounterLabel;

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
        GtkStack*  stack         = ((StackState*)user_data)->stack;
        GtkWidget* settings_page = ((StackState*)user_data)->page;
        gtk_stack_set_visible_child(stack, settings_page);
}

void on_back_button_clicked(GtkWidget* widget, gpointer user_data)
{
        (void)widget;
        GtkStack*  stack     = ((StackState*)user_data)->stack;
        GtkWidget* main_page = ((StackState*)user_data)->page;
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
                        webkit_web_view_load_uri(web_view, parsed_uri.str);
                } else {
                        S_
                                char* search_url = str_to_brave_search_url(parsed_uri.str);
                                defer(dg_free, search_url);
                                webkit_web_view_load_uri(web_view, search_url);
                        _S
                }
        _S
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
        webkit_web_view_get_type();
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

        GtkWindow*    window     = GTK_WINDOW(BUILDER_GET_OBJECT(builder, AdwWindow, ADW_WINDOW, "window_main"));
        GtkStack*     stack_main = BUILDER_GET_OBJECT(builder, GtkStack, GTK_STACK, "stack_main");
        GtkStackPage* main_stack = BUILDER_GET_OBJECT(builder, GtkStackPage, GTK_STACK_PAGE, "main_page");
        GtkStackPage* main_page  = BUILDER_GET_OBJECT(builder, GtkStackPage, GTK_STACK_PAGE, "main_page");

        GtkStackPage* settings_page = BUILDER_GET_OBJECT(builder, GtkStackPage, GTK_STACK_PAGE, "settings_page");
        GtkWidget*    template      = BUILDER_GET_OBJECT(builder, GtkWidget, GTK_WIDGET, "settings_page_template");

        GtkWidget* back_button = template_app_settings_page_get_back_button(TEMPLATE_APP_SETTINGS_PAGE(template));
        check_gobject(G_OBJECT(back_button), "Error: Failed to get the back_button.\n");
        GtkWidget* open_settings_button = GTK_WIDGET(gtk_builder_get_object(builder, "open_settings_button"));
        check_gobject(G_OBJECT(open_settings_button), "Error: Failed to get the open_settings_button.\n");

        web_view = WEBKIT_WEB_VIEW(gtk_builder_get_object(builder, "web_view"));
        check_gobject(G_OBJECT(web_view), "Error: Failed to get the web_view.\n");
        webkit_web_view_load_uri(web_view, "https://search.brave.com/");

        open_settings_state.stack = stack_main;
        open_settings_state.page  = gtk_stack_page_get_child(settings_page);

        back_to_main_state.stack = stack_main;
        back_to_main_state.page  = gtk_stack_page_get_child(main_page);

        g_signal_connect(open_settings_button, "clicked", G_CALLBACK(on_open_settings_button_clicked), &open_settings_state);
        g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), &back_to_main_state);

        gtk_window_set_application(window, GTK_APPLICATION(app));
        gtk_window_present(window);
}

int main(int argc, char* argv[])
{
        if (argc >= 2) {
                if (0 == strcmp(argv[1], "--version"))
                        version();
        }

        uri_init();
        app = adw_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);

        if (builder)
                g_object_unref(builder);
        g_object_unref(app);
        uri_cleanup();

        return status;
}