#ifndef BINBOX_H
#define BINBOX_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ASCETIC_TYPE_BINBOX ascetic_binbox_get_type()
G_DECLARE_FINAL_TYPE(AsceticBinbox, ascetic_binbox, ASCETIC, BINBOX, GtkWidget);

AsceticBinbox* ascetic_binbox_new(void);

G_END_DECLS

#endif // BINBOX_H