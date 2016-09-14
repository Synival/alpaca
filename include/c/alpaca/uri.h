/* uri.h
 * -----
 * URI interpretation and break-down for requests. */

#ifndef __ALPACA_C_URI_H
#define __ALPACA_C_URI_H

#include "defs.h"

/* URIs passed to HTTP functions. */
struct _al_uri_t {
   al_flags_t flags;

   /* strings, broken down to full path and query. */
   char *str_full, *str_path, *str_query;
   al_uri_path_t *path;
   al_uri_parameter_t *parameters;
};

/* a chain of locations in a path. */
struct _al_uri_path_t {
   char *name;
   al_uri_t *uri;
   al_uri_path_t *prev, *next;
};

/* parameters extrapolated from the query string. */
struct _al_uri_parameter_t {
   char *name, *value;
   al_uri_t *uri;
   al_uri_parameter_t *prev, *next;
};

/* URI functions. */
al_uri_t *al_uri_new (const char *string);
int al_uri_free (al_uri_t *uri);
char *al_uri_decode (const char *input, char *output, size_t output_size);
al_uri_path_t *al_uri_path_append (al_uri_t *uri, al_uri_path_t *prev,
   char *name);
int al_uri_path_free (al_uri_path_t *path);
al_uri_parameter_t *al_uri_parameter_append (al_uri_t *uri,
   al_uri_parameter_t *prev, char *name, char *value);
int al_uri_parameter_free (al_uri_parameter_t *param);

#endif
