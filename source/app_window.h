#ifndef APP_WINDOW_H
#define APP_WINDOW_H

#include <gtk/gtk.h>
#include <adwaita.h>
#include <webkit/webkit.h>
#include "settings_page.h"

G_BEGIN_DECLS

#define ASCETIC_TYPE_APP_WINDOW ascetic_app_window_get_type()
G_DECLARE_FINAL_TYPE(AsceticAppWindow, ascetic_app_window, ASCETIC, APP_WINDOW, AdwApplicationWindow)

AsceticAppWindow* ascetic_app_window_new(void);
AdwTabPage*       new_tab(GtkWidget* widget, gpointer user_data);

G_END_DECLS

#endif // APP_WINDOW_H