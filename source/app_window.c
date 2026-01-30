#include "app_window.h"
#include "globals.h"
#include "webview.h"
#include "settings_page.h"
#include <adwaita.h>
#include <webkit/webkit.h>

GObject* _builder_get_object(GtkBuilder* builder, char* name)
{
        GObject* obj = gtk_builder_get_object(builder, name);
#if DEBUG
        if (!obj) {
                g_printerr("Error: Failed to get %s from builder", name);
                g_application_quit(G_APPLICATION(app));
        }
#endif
        return obj;
}

#define builder_get_object(builder, TYPE, name) TYPE(_builder_get_object(builder, name))

G_DEFINE_TYPE(AsceticAppWindow, ascetic_app_window, ADW_TYPE_APPLICATION_WINDOW)

static void ascetic_app_window_dispose(GObject* object);

static void ascetic_app_window_class_init(AsceticAppWindowClass* klass)
{
        G_OBJECT_CLASS(klass)->dispose = ascetic_app_window_dispose;
        gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), APP_PREFIX "/app_window.ui");
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, stack_main);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, web_page);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, settings_page);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, web_tab_view);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, web_tab_bar);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, url_entry);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, revealer_main_toolbar);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, open_settings_button);
        gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), AsceticAppWindow, upper_new_tab_button);
}

void on_tab_bar_visibility_changed(AdwTabBar* tab_bar, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        GtkWidget* upper_new_tab_button = GTK_WIDGET(user_data);
        gboolean   tabs_revealed        = adw_tab_bar_get_tabs_revealed(tab_bar);
        if (tabs_revealed) {
                gtk_widget_set_opacity(upper_new_tab_button, 0.0);
                gtk_widget_set_sensitive(upper_new_tab_button, FALSE);
        } else {
                gtk_widget_set_opacity(upper_new_tab_button, 1.0);
                gtk_widget_set_sensitive(upper_new_tab_button, TRUE);
        }
}

static void ascetic_app_window_init(AsceticAppWindow* self)
{
        gtk_widget_init_template(GTK_WIDGET(self));
        self->close_settings_button = GTK_BUTTON(ascetic_settings_page_get_close_settings_button(self->settings_page));
}

static void ascetic_app_window_dispose(GObject* object)
{
        gtk_widget_dispose_template(GTK_WIDGET(object), ASCETIC_TYPE_APP_WINDOW);
        G_OBJECT_CLASS(ascetic_app_window_parent_class)->dispose(object);
}