#include "data.h"
#include "settings_page.h"
#include "version.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The macros VERSION, NAME, TARGET, AUTHOR and COPYRIGHT are available for this program to reference
information about itself. These macros are defined in config.mk */

static GtkBuilder*     builder;
static GtkApplication* app;

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

typedef struct {
        GtkLabel* label;
        int       value;
        char      text[32];
} CounterLabel;

void connect_button_to_label(gchar* button_name, gchar* label_name, CounterLabel* state, GtkBuilder* builder)
{
        GObject* button = gtk_builder_get_object(builder, button_name);
        check_gobject(button, NULL);
        GObject* label = gtk_builder_get_object(builder, label_name);
        check_gobject(label, NULL);
        state->label = GTK_LABEL(label);
        g_object_set_data(button, "state", state);
}

static CounterLabel label_1_state;
static CounterLabel label_2_state;

void on_button_1_clicked(GtkWidget* widget, gpointer data)
{
        (void)data;
        CounterLabel* state = g_object_get_data(G_OBJECT(widget), "state");
        state->value++;
        snprintf(state->text, sizeof(state->text), "%d", state->value);
        gtk_label_set_label(state->label, state->text);
}

void on_button_2_clicked(GtkWidget* widget, gpointer data)
{
        (void)widget;
        CounterLabel* state = (CounterLabel*)data;
        state->value++;
        snprintf(state->text, sizeof(state->text), "%d", state->value);
        gtk_label_set_label(state->label, state->text);
}

typedef struct {
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

void activate(GtkApplication* app, gpointer user_data)
{
        (void)user_data;
        GtkBuilderScope* scope = gtk_builder_cscope_new();
        gtk_builder_cscope_add_callback(scope, on_button_1_clicked);

        /* If you register signals in the xml/blueprint, you also must now register */
        /* your callbacks via a new scope system in GTK 4. */
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
        /*
        If you manage signals/callbacks entirely in your code source files, it's
        a simple one liner to initialize the builder:

        builder = gtk_builder_new_from_resource(APP_PREFIX "/window_main.ui");
        */

        GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "window_main"));
        check_gobject(G_OBJECT(window), "Error: Failed to get the main window.\n");
        GtkStack* stack_main = GTK_STACK(gtk_builder_get_object(builder, "stack_main"));
        check_gobject(G_OBJECT(stack_main), "Error: Failed to get the main stack.\n");

        GtkStackPage* main_page = GTK_STACK_PAGE(gtk_builder_get_object(builder, "main_page"));
        check_gobject(G_OBJECT(main_page), "Error: Failed to get the main_page.\n");
        GtkStackPage* settings_page = GTK_STACK_PAGE(gtk_builder_get_object(builder, "settings_page"));
        check_gobject(G_OBJECT(settings_page), "Error: Failed to get the settings_page.\n");
        GtkWidget* template = GTK_WIDGET(gtk_builder_get_object(builder, "settings_page_template"));
        check_gobject(G_OBJECT(template), "Error: Failed to get the settings_page_template.\n");

        // Two ways to get the back_button:
        //GtkWidget* back_button = GTK_WIDGET(gtk_widget_get_template_child(template, TEMPLATE_APP_TYPE_SETTINGS_PAGE, "back_button"));
        GtkWidget* back_button = template_app_settings_page_get_back_button(TEMPLATE_APP_SETTINGS_PAGE(template));

        check_gobject(G_OBJECT(back_button), "Error: Failed to get the back_button.\n");
        GtkWidget* open_settings_button = GTK_WIDGET(gtk_builder_get_object(builder, "open_settings_button"));
        check_gobject(G_OBJECT(open_settings_button), "Error: Failed to get the open_settings_button.\n");

        open_settings_state.stack = stack_main;
        open_settings_state.page  = gtk_stack_page_get_child(settings_page);

        back_to_main_state.stack = stack_main;
        back_to_main_state.page  = gtk_stack_page_get_child(main_page);

        g_signal_connect(open_settings_button, "clicked", G_CALLBACK(on_open_settings_button_clicked), &open_settings_state);
        g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), &back_to_main_state);

        gtk_window_set_application(window, app);
        gtk_window_present(window);

        connect_button_to_label("counter_1", "label_1", &label_1_state, builder);
        connect_button_to_label("counter_2", "label_2", &label_2_state, builder);

        GObject* button_2 = gtk_builder_get_object(builder, "counter_2");

        g_signal_connect(button_2, "clicked", G_CALLBACK(on_button_2_clicked), &label_2_state);
}

int main(int argc, char* argv[])
{
        if (argc >= 2) {
                if (0 == strcmp(argv[1], "--version"))
                        version();
        }

        app = gtk_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
        int status = g_application_run(G_APPLICATION(app), argc, argv);

        if (builder)
                g_object_unref(builder);
        g_object_unref(app);

        return status;
}