#include "settings_page.h"
#include <gtk/gtk.h>

struct _AsceticSettingsPage {
        GtkWidget  parent_instance;
        GtkWidget* close_settings_button;
};

GtkWidget* ascetic_settings_page_get_close_settings_button(AsceticSettingsPage* self)
{
        return self->close_settings_button;
}

G_DEFINE_TYPE(AsceticSettingsPage, ascetic_settings_page, GTK_TYPE_WIDGET)

static void ascetic_settings_page_dispose(GObject* object);

static void ascetic_settings_page_class_init(AsceticSettingsPageClass* klass)
{
        G_OBJECT_CLASS(klass)->dispose = ascetic_settings_page_dispose;
        gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), APP_PREFIX "/settings_page.ui");
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticSettingsPage, close_settings_button);
        gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BIN_LAYOUT);
}

static void ascetic_settings_page_init(AsceticSettingsPage* self)
{
        gtk_widget_init_template(GTK_WIDGET(self));
}

static void ascetic_settings_page_dispose(GObject* object)
{
        AsceticSettingsPage* self  = ASCETIC_SETTINGS_PAGE(object);
        GtkWidget*               child = gtk_widget_get_first_child(GTK_WIDGET(self));
        if (child) {
                gtk_widget_unparent(child);
        }
        gtk_widget_dispose_template(GTK_WIDGET(self), ASCETIC_TYPE_SETTINGS_PAGE);

        G_OBJECT_CLASS(ascetic_settings_page_parent_class)->dispose(object);
}
