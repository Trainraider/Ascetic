#include "settings_page.h"
#include <gtk/gtk.h>

struct _TemplateAppSettingsPage {
        GtkWidget  parent_instance;
        GtkWidget* back_button;
};

GtkWidget* template_app_settings_page_get_back_button(TemplateAppSettingsPage* self)
{
        return self->back_button;
}

G_DEFINE_TYPE(TemplateAppSettingsPage, template_app_settings_page, GTK_TYPE_WIDGET)

static void template_app_settings_page_dispose(GObject* object);

static void template_app_settings_page_class_init(TemplateAppSettingsPageClass* klass)
{
        G_OBJECT_CLASS(klass)->dispose = template_app_settings_page_dispose;
        gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), APP_PREFIX "/settings_page.ui");
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), TemplateAppSettingsPage, back_button);
        gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BIN_LAYOUT);
}

static void template_app_settings_page_init(TemplateAppSettingsPage* self)
{
        gtk_widget_init_template(GTK_WIDGET(self));
}

static void template_app_settings_page_dispose(GObject* object)
{
        TemplateAppSettingsPage* self  = TEMPLATE_APP_SETTINGS_PAGE(object);
        GtkWidget*               child = gtk_widget_get_first_child(GTK_WIDGET(self));
        if (child) {
                gtk_widget_unparent(child);
        }
        gtk_widget_dispose_template(GTK_WIDGET(self), TEMPLATE_APP_TYPE_SETTINGS_PAGE);

        G_OBJECT_CLASS(template_app_settings_page_parent_class)->dispose(object);
}
