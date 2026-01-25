#include "binbox.h"
#include <gtk/gtk.h>

struct _AsceticBinbox {
        GtkWidget parent_instance;
};

G_DEFINE_TYPE(AsceticBinbox, ascetic_binbox, GTK_TYPE_WIDGET)

static void ascetic_binbox_class_dispose(GObject* object);

static void ascetic_binbox_class_init(AsceticBinboxClass* klass)
{
        G_OBJECT_CLASS(klass)->dispose = ascetic_binbox_class_dispose;
        gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(klass), APP_PREFIX "/binbox.ui");
        gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BIN_LAYOUT);
}

static void ascetic_binbox_init(AsceticBinbox* self)
{
        gtk_widget_init_template(GTK_WIDGET(self));
}
static void ascetic_binbox_class_dispose(GObject* object)
{
        AsceticBinbox* self  = ASCETIC_BINBOX(object);
        GtkWidget*     child = gtk_widget_get_first_child(GTK_WIDGET(self));
        if (child) {
                gtk_widget_unparent(child);
        }
        gtk_widget_dispose_template(GTK_WIDGET(self), ASCETIC_TYPE_BINBOX);

        G_OBJECT_CLASS(ascetic_binbox_parent_class)->dispose(object);
}