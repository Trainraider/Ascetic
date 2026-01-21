#include "uri.h"
#include <glib-2.0/glib.h>
#include <stdbool.h>
#include <uriparser/Uri.h>

bool uri_has_scheme(const char* uri)
{
        if (g_strcmp0(uri, "") == 0) {
                return false;
        }

        if (g_str_has_prefix(uri, "http://") || g_str_has_prefix(uri, "https://") || g_str_has_prefix(uri, "file://")) {
                return true;
        }

        return false;
}

bool uri_is_valid(const char* uri)
{
        UriUriA     parsed_uri;
        const char* errorPos;
        if (uriParseSingleUriA(&parsed_uri, uri, &errorPos) != URI_SUCCESS) {
                return false;
        }
        uriFreeUriMembersA(&parsed_uri);
        return true;
}

ParsedUri uri_parse(const char* input)
{
        ParsedUri result = { 0 };

        if (!uri_has_scheme(input)) {
                result.str  = g_strdup_printf("http://%s", input);
                bool is_uri = uri_is_valid(result.str);
                if (!is_uri) {
                        g_free(result.str);
                        result.str = g_strdup(input);
                }
                result.is_uri = is_uri;
                return result;
        }
        result.str    = g_strdup(input);
        result.is_uri = uri_is_valid(result.str);
        return result;
}

char* str_to_google_search_url(const char* query)
{
        if (!query)
                return NULL;

        char* encoded = g_uri_escape_string(query, NULL, false);
        char* url     = g_strdup_printf("https://www.google.com/search?q=%s", encoded);
        g_free(encoded);

        return url;
}