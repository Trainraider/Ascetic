#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <webkit/webkit.h>
#include <adwaita.h>
#include "globals.h"

void browser_session_init(void);

void browser_session_cleanup(void);

WebKitWebView* create_webview(WebKitWebView* related_view);
void           on_webview_title_changed(WebKitWebView* webview, GParamSpec* pspec, gpointer user_data);
WebKitWebView* tab_get_webview(AdwTabPage* tab);

#endif // WEBVIEW_H