#include <gtk/gtk.h>
#include <adwaita.h>
#include <webkit/webkit.h>
#include "app_window.h"

extern AdwApplication*   app;
extern GIcon*            new_tab_icon;
extern GHashTable*       app_windows;
extern AsceticAppWindow* active_window;