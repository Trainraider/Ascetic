#ifndef URI_H
#define URI_H

#include <stdbool.h>

typedef struct {
        char* str;
        bool  is_uri;
} ParsedUri;

bool      uri_has_scheme(const char* uri);
bool      uri_is_valid(const char* uri);
ParsedUri uri_parse(const char* input);
char*     str_to_google_search_url(const char* query);

#endif // URI_H