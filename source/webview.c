#include "webview.h"
#include "app_window.h"
#include "defer.h"

// Singleton for shared browser session components
static struct {
        WebKitNetworkSession*         session;
        WebKitWebContext*             context;
        WebKitSettings*               settings;
        WebKitUserContentManager*     content_manager;
        WebKitUserContentFilterStore* filter_store;
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

static void on_filter_saved(GObject* source, GAsyncResult* result, gpointer user_data)
{
        GError*                  error  = NULL;
        WebKitUserContentFilter* filter = webkit_user_content_filter_store_save_finish(
            WEBKIT_USER_CONTENT_FILTER_STORE(source), result, &error);
        webkit_user_content_manager_add_filter(WEBKIT_USER_CONTENT_MANAGER(user_data), filter);
        webkit_user_content_filter_unref(filter);
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

                WebKitWebsiteDataManager* data_manager = webkit_network_session_get_website_data_manager(browser_session.session);
                webkit_website_data_manager_set_favicons_enabled(data_manager, TRUE);

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
                webkit_settings_set_user_agent(browser_session.settings,
                    "Mozilla/5.0 (Macintosh; Intel Mac OS X 15_7_4) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/26.0 Safari/605.1.15");
                webkit_settings_set_enable_media(browser_session.settings, FALSE);
                webkit_settings_set_enable_media_stream(browser_session.settings, FALSE);

                browser_session.content_manager = webkit_user_content_manager_new();

                local char* filter_dir = g_build_filename(data_dir, "filters", NULL);
                defer(dg_free, filter_dir);
                g_mkdir_with_parents(filter_dir, 0700);
                browser_session.filter_store = webkit_user_content_filter_store_new(filter_dir);

                GBytes* rules = g_resources_lookup_data(
                    APP_PREFIX "/block-images.json", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
                webkit_user_content_filter_store_save(
                    browser_session.filter_store,
                    "block-images",
                    rules,
                    NULL,
                    on_filter_saved,
                    browser_session.content_manager);
                g_bytes_unref(rules);
        _S
}

void browser_session_cleanup(void)
{
        g_object_unref(browser_session.filter_store);
        g_object_unref(browser_session.content_manager);
        g_object_unref(browser_session.settings);
        g_object_unref(browser_session.session);
}

void on_load_changed_update_background(WebKitWebView* webview, WebKitLoadEvent load_event, gpointer user_data)
{
        switch (load_event) {
        case WEBKIT_LOAD_STARTED:
                webkit_web_view_set_background_color(webview, &((GdkRGBA) { 0.1, 0.1, 0.1, 1.0 }));
                break;
        case WEBKIT_LOAD_FINISHED:
                webkit_web_view_set_background_color(webview, &((GdkRGBA) { 1.0, 1.0, 1.0, 1.0 }));
                break;
        default:
                break;
        }
}

WebKitWebView* create_webview(WebKitWebView* related_view)
{
        WebKitWebView* webview;
        if (related_view) {
                webview = WEBKIT_WEB_VIEW(g_object_new(
                    WEBKIT_TYPE_WEB_VIEW,
                    "settings", browser_session.settings,
                    "related-view", related_view,
                    "user-content-manager", browser_session.content_manager,
                    "hexpand", TRUE,
                    "vexpand", TRUE,
                    NULL));
        } else {
                webview = WEBKIT_WEB_VIEW(g_object_new(
                    WEBKIT_TYPE_WEB_VIEW,
                    "network-session", browser_session.session,
                    "web-context", browser_session.context,
                    "settings", browser_session.settings,
                    "user-content-manager", browser_session.content_manager,
                    "hexpand", TRUE,
                    "vexpand", TRUE,
                    NULL));
        }

        webkit_web_view_set_background_color(webview, &((GdkRGBA) { 0.1, 0.1, 0.1, 1.0 }));
        g_signal_connect(webview, "load-changed", G_CALLBACK(on_load_changed_update_background), NULL);
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

void on_webview_favicon_changed(WebKitWebView* webview, GParamSpec* pspec, gpointer user_data)
{
        (void)pspec;
        AdwTabPage* tab     = ADW_TAB_PAGE(user_data);
        GdkTexture* favicon = webkit_web_view_get_favicon(webview);
        if (favicon) {
                adw_tab_page_set_icon(tab, G_ICON(favicon));
        } else {
                adw_tab_page_set_icon(tab, new_tab_icon);
        }
}

WebKitWebView* tab_get_webview(AdwTabPage* tab)
{
        return WEBKIT_WEB_VIEW(adw_tab_page_get_child(tab));
}
