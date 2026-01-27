#include "webview.h"
#include "defer.h"

// Singleton for shared browser session components
static struct {
        WebKitNetworkSession* session;
        WebKitWebContext*     context;
        WebKitSettings*       settings;
} browser_session = { 0 };

nonlocal char* get_config_dir()
{
        S_
                external const char* config_base = g_get_user_config_dir();
                nonlocal char*       config_dir  = g_build_filename(config_base, TARGET, NULL);
                errdefer(dg_free, config_dir);

                if (g_mkdir_with_parents(config_dir, 0700)) {
                        g_warning("Error: Failed to create config directory: %s\n", config_dir);
                        returnerr NULL;
                }

                return config_dir;
        _S
}

nonlocal char* get_data_dir()
{
        S_
                external const char* data_base = g_get_user_data_dir();
                nonlocal char*       data_dir  = g_build_filename(data_base, TARGET, NULL);
                errdefer(dg_free, data_dir);

                if (g_mkdir_with_parents(data_dir, 0700)) {
                        g_warning("Error: Failed to create data directory: %s\n", data_dir);
                        returnerr NULL;
                }

                return data_dir;
        _S
}

nonlocal char* get_cache_dir()
{
        S_
                external const char* cache_base = g_get_user_cache_dir();
                nonlocal char*       cache_dir  = g_build_filename(cache_base, TARGET, NULL);
                errdefer(dg_free, cache_dir);

                if (g_mkdir_with_parents(cache_dir, 0700)) {
                        g_warning("Error: Failed to create cache directory: %s\n", cache_dir);
                        returnerr NULL;
                }

                return cache_dir;
        _S
}

void browser_session_init(void)
{
        S_
                // null dirs are handled gracefully by webkit
                // no error checking needed
                local char* data_dir = get_data_dir();
                defer(dg_free, data_dir);
                local char* cache_dir = get_cache_dir();
                defer(dg_free, cache_dir);

                browser_session.session = webkit_network_session_new(data_dir, cache_dir);

                WebKitCookieManager* cookie_manager = webkit_network_session_get_cookie_manager(browser_session.session);
                local char*          cookie_file    = g_build_filename(data_dir, "cookies.sqlite", NULL);
                defer(dg_free, cookie_file);
                webkit_cookie_manager_set_persistent_storage(
                    cookie_manager,
                    cookie_file,
                    WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
                webkit_cookie_manager_set_accept_policy(
                    cookie_manager,
                    WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);

                webkit_network_session_set_persistent_credential_storage_enabled(
                    browser_session.session, TRUE);

                browser_session.context  = webkit_web_context_get_default();
                browser_session.settings = webkit_settings_new();
        _S
}

void browser_session_cleanup(void)
{
        g_object_unref(browser_session.settings);
        g_object_unref(browser_session.session);
}

WebKitWebView* create_webview(WebKitWebView* related_view)
{
        WebKitWebView* webview;
        if (related_view) {
                webview = WEBKIT_WEB_VIEW(g_object_new(
                    WEBKIT_TYPE_WEB_VIEW,
                    "settings", browser_session.settings,
                    "related-view", related_view,
                    "hexpand", TRUE,
                    "vexpand", TRUE,
                    NULL));
        } else {
                webview = WEBKIT_WEB_VIEW(g_object_new(
                    WEBKIT_TYPE_WEB_VIEW,
                    "network-session", browser_session.session,
                    "web-context", browser_session.context,
                    "settings", browser_session.settings,
                    "hexpand", TRUE,
                    "vexpand", TRUE,
                    NULL));
        }

        webkit_web_view_set_background_color(webview, &((GdkRGBA) { 0.1, 0.1, 0.1, 1.0 }));
        g_signal_connect(webview, "create", G_CALLBACK(on_create_web_view), NULL);
        return webview;
}

void on_webview_title_changed(WebKitWebView* webview, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        AdwTabPage* tab   = ADW_TAB_PAGE(user_data);
        const char* title = webkit_web_view_get_title(webview);
        if (title) {
                adw_tab_page_set_title(tab, title);
        } else {
                adw_tab_page_set_title(tab, "New Tab");
        }
}

void on_webview_uri_changed(WebKitWebView* webview, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        AdwTabPage* tab          = ADW_TAB_PAGE(user_data);
        AdwTabPage* selected_tab = adw_tab_view_get_selected_page(tab_view);
        if (tab != selected_tab)
                return;
        if (gtk_widget_has_focus(GTK_WIDGET(url_entry)))
                return;
        const char*     uri          = webkit_web_view_get_uri(webview);
        GtkEntryBuffer* entry_buffer = gtk_entry_get_buffer(url_entry);
        if (uri) {
                gtk_entry_buffer_set_text(entry_buffer, uri, -1);
        } else {
                gtk_entry_buffer_set_text(entry_buffer, "", -1);
        }
}

AdwTabPage* new_tab(GtkWidget* widget, gpointer user_data)
{
        (void)user_data;
        WebKitWebView* related_webview = NULL;

        if (G_TYPE_CHECK_INSTANCE_TYPE(widget, WEBKIT_TYPE_WEB_VIEW)) {
                related_webview = WEBKIT_WEB_VIEW(widget);
        }
        WebKitWebView* webview = create_webview(related_webview);
        AdwTabPage*    tab     = adw_tab_view_append(tab_view, GTK_WIDGET(webview));
        adw_tab_page_set_title(tab, "New Tab");
        adw_tab_page_set_icon(tab, new_tab_icon);
        g_signal_connect(webview, "notify::title", G_CALLBACK(on_webview_title_changed), tab);
        g_signal_connect(webview, "notify::uri", G_CALLBACK(on_webview_uri_changed), tab);
        if (G_TYPE_CHECK_INSTANCE_TYPE(widget, GTK_TYPE_BUTTON)) {
                gtk_widget_grab_focus(GTK_WIDGET(url_entry));
        }
        return tab;
}

WebKitWebView* on_create_web_view(WebKitWebView* webview, WebKitNavigationAction* navigation_action, gpointer user_data)
{
        (void)navigation_action;
        (void)user_data;
        AdwTabPage* tab = new_tab(GTK_WIDGET(webview), NULL);
        return tab_get_webview(tab);
}

void on_tab_changed(GObject* self, GParamSpec* pspec, gpointer user_data)
{
        (void)self;
        (void)pspec;
        (void)user_data;
        AdwTabPage* selected_page = adw_tab_view_get_selected_page(tab_view);
        if (!selected_page) {
                active_web_view = NULL;
                GtkEntryBuffer* entry_buffer = gtk_entry_get_buffer(url_entry);
                gtk_entry_buffer_set_text(entry_buffer, "", -1);
                return;
        }
        active_web_view = tab_get_webview(selected_page);
        // update url entry
        const char*     uri          = webkit_web_view_get_uri(active_web_view);
        GtkEntryBuffer* entry_buffer = gtk_entry_get_buffer(url_entry);
        if (uri) {
                gtk_entry_buffer_set_text(entry_buffer, uri, -1);
        } else {
                gtk_entry_buffer_set_text(entry_buffer, "", -1);
        }
}

WebKitWebView* tab_get_webview(AdwTabPage* tab)
{
        return WEBKIT_WEB_VIEW(adw_tab_page_get_child(tab));
}
