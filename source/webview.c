#include <webkit/webkit.h>
#include "defer.h"

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

WebKitWebView* create_webview()
{
        S_
                local char* data_dir = get_data_dir();
                defer(dg_free, data_dir);
                local char* cache_dir = get_cache_dir();
                defer(dg_free, cache_dir);
                WebKitNetworkSession* session = webkit_network_session_new(
                    data_dir,
                    cache_dir
                );
                
                // Configure cookie manager to persist cookies
                WebKitCookieManager* cookie_manager = webkit_network_session_get_cookie_manager(session);
                local char* cookie_file = g_build_filename(data_dir, "cookies.sqlite", NULL);
                defer(dg_free, cookie_file);
                webkit_cookie_manager_set_persistent_storage(
                    cookie_manager,
                    cookie_file,
                    WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
                webkit_cookie_manager_set_accept_policy(
                    cookie_manager,
                    WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS);

                // Enable persistent credential storage
                webkit_network_session_set_persistent_credential_storage_enabled(session, TRUE);

                WebKitWebContext* context  = webkit_web_context_get_default();
                WebKitSettings*   settings = webkit_settings_new();

                WebKitWebView* webview = WEBKIT_WEB_VIEW(g_object_new(
                    WEBKIT_TYPE_WEB_VIEW,
                    "network-session", session,
                    "web-context", context,
                    "settings", settings,
                    NULL));

                return webview;
        _S
}