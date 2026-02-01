#include "defer.h"
#include "uri.h"
#include <ada_c.h>
#include <glib-2.0/glib.h>
#include <libpsl.h>

#define ADA_HOST_TYPE_DEFAULT 0
#define ADA_HOST_TYPE_IPV4 1
#define ADA_HOST_TYPE_IPV6 2

static psl_ctx_t* psl;

gboolean uri_has_scheme(const char* uri)
{
        if (g_strcmp0(uri, "") == 0) {
                return FALSE;
        }

        if (g_str_has_prefix(uri, "http://") || g_str_has_prefix(uri, "https://") || g_str_has_prefix(uri, "file://")) {
                return TRUE;
        }

        return FALSE;
}

void dada_free(void* ptr)
{
        void** p = (void**)ptr;
        if (*p) {
                ada_free(*p);
        }
}

gboolean uri_is_navigable(const char* uri)
{
        S_
                ada_url url = ada_parse(uri, strlen(uri));
                defer(dada_free, url);

                if (!ada_is_valid(url)) {
                        return FALSE;
                }

                uint8_t host_type = ada_get_host_type(url);

                if (host_type == ADA_HOST_TYPE_IPV4 || host_type == ADA_HOST_TYPE_IPV6) {
                        return TRUE;
                }

                ada_string hostname = ada_get_hostname(url);

                char* hostname_str = malloc(hostname.length + 1);
                defer(dfree, hostname_str);
                memcpy(hostname_str, hostname.data, hostname.length);
                hostname_str[hostname.length] = '\0';

                if (!g_strcmp0(hostname_str, "localhost")) {
                        return TRUE;
                }

                const char* reg_domain = psl_registrable_domain(psl, hostname_str);

                return reg_domain != NULL;
        _S
}

void uri_init(void)
{
        psl = (psl_ctx_t*)psl_builtin();
}

void uri_cleanup(void)
{
        psl_free(psl);
}

ParsedUri uri_parse(const char* input)
{
        ParsedUri result = { 0 };

        if (!uri_has_scheme(input)) {
                result.str            = g_strdup_printf("http://%s", input);
                gboolean is_navigable = uri_is_navigable(result.str);

                if (!is_navigable) {
                        g_free(result.str);
                        result.str = g_strdup(input);
                }
                result.is_uri = is_navigable;
                return result;
        }

        result.str    = g_strdup(input);
        result.is_uri = TRUE;
        return result;
}

char* str_to_brave_search_url(const char* query)
{
        if (!query)
                return NULL;

        char* encoded = g_uri_escape_string(query, NULL, FALSE);
        char* url     = g_strdup_printf("https://search.brave.com/search?q=%s", encoded);
        g_free(encoded);

        return url;
}