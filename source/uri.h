#ifndef URI_H
#define URI_H

#include <stdbool.h>

typedef struct {
        char* str;
        bool  is_uri;
} ParsedUri;

void      uri_init(void);
void      uri_cleanup(void);
bool      uri_has_scheme(const char* uri);
ParsedUri uri_parse(const char* input);
char*     str_to_brave_search_url(const char* query);

#endif // URI_H