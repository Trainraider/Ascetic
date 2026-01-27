#include <gtk/gtk.h>
#include <adwaita.h>
#include <webkit/webkit.h>

extern GtkBuilder*     builder;
extern AdwApplication* app;
extern AdwTabOverview* tab_overview;
extern AdwTabView*     tab_view;
extern AdwTabBar*      tab_bar;
extern WebKitWebView*  active_web_view;
extern GIcon*          new_tab_icon;
extern GtkEntry*       url_entry;