#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <webkit/webkit.h>
#include <adwaita.h>
#include "globals.h"

typedef struct BrowserTab BrowserTab;

void browser_session_init(void);

void browser_session_cleanup(void);

WebKitWebView* create_webview(WebKitWebView* related_view);
AdwTabPage*    new_tab(GtkWidget* widget, gpointer user_data);
void           on_tab_changed(GObject* self, GParamSpec* pspec, gpointer user_data);
WebKitWebView* tab_get_webview(AdwTabPage* tab);
WebKitWebView* on_create_web_view(WebKitWebView* webview, WebKitNavigationAction* navigation_action, gpointer user_data);

#endif // WEBVIEW_H