#include "settings_page.h"
#include "app_window.h"
#include <gtk/gtk.h>

struct _AsceticSettingsPage {
        GtkWidget  parent_instance;
        GtkWidget* close_settings_button;
};

G_DEFINE_TYPE(AsceticSettingsPage, ascetic_settings_page, GTK_TYPE_WIDGET)

enum {
        CLOSE_REQUESTED,
        LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

void on_close_settings_button_clicked(GtkWidget* widget, gpointer user_data)
{
        AsceticSettingsPage* self = ASCETIC_SETTINGS_PAGE(user_data);
        g_signal_emit(self, signals[CLOSE_REQUESTED], 0);
}

static void ascetic_settings_page_dispose(GObject* object);

static void ascetic_settings_page_class_init(AsceticSettingsPageClass* klass)
{
        G_OBJECT_CLASS(klass)->dispose = ascetic_settings_page_dispose;
        signals[CLOSE_REQUESTED] = g_signal_new(
                "close-requested",
                G_TYPE_FROM_CLASS(klass),
                G_SIGNAL_RUN_LAST,
                0,
                NULL,
                NULL,
                NULL,
                G_TYPE_NONE,
                0
        );
        gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), APP_PREFIX "/settings_page.ui");
        // Uncomment when using binding signals in the blueprint:
        // GtkBuilderScope* scope = gtk_builder_cscope_new();
        // gtk_builder_cscope_add_callback(scope, on_close_settings_button_clicked);
        // gtk_widget_class_set_template_scope(GTK_WIDGET_CLASS(klass), scope);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticSettingsPage, close_settings_button);
        gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BIN_LAYOUT);
}

static void ascetic_settings_page_init(AsceticSettingsPage* self)
{
        gtk_widget_init_template(GTK_WIDGET(self));
        g_signal_connect(self->close_settings_button, "clicked", G_CALLBACK(on_close_settings_button_clicked), self);
}

static void ascetic_settings_page_dispose(GObject* object)
{
        AsceticSettingsPage* self  = ASCETIC_SETTINGS_PAGE(object);
        GtkWidget*           child = gtk_widget_get_first_child(GTK_WIDGET(self));
        if (child) {
                gtk_widget_unparent(child);
        }
        gtk_widget_dispose_template(GTK_WIDGET(self), ASCETIC_TYPE_SETTINGS_PAGE);

        G_OBJECT_CLASS(ascetic_settings_page_parent_class)->dispose(object);
}
