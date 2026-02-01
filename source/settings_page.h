#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ASCETIC_TYPE_SETTINGS_PAGE ascetic_settings_page_get_type()
G_DECLARE_FINAL_TYPE(AsceticSettingsPage, ascetic_settings_page, ASCETIC, SETTINGS_PAGE, GtkWidget)

AsceticSettingsPage* ascetic_settings_page_new(void);

G_END_DECLS

#endif // SETTINGS_PAGE_H