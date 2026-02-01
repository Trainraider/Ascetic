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

G_END_DECLS

struct _AsceticAppWindow {
        AdwApplicationWindow parent_instance;
        GtkStack*            stack_main;
        AdwTabOverview*      web_page;
        AsceticSettingsPage* settings_page;
        AdwTabView*          web_tab_view;
        AdwTabBar*           web_tab_bar;
        WebKitWebView*       active_web_view;
        GtkEntry*            url_entry;
        GtkRevealer*         revealer_main_toolbar;
        GtkButton*           open_settings_button;
        GtkButton*           upper_new_tab_button;
};

#endif // APP_WINDOW_H