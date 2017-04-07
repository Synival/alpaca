/* uri.h
 * -----
 * URI interpretation and break-down for requests. */

#ifndef __ALPACA_C_URI_H
#define __ALPACA_C_URI_H

#include "defs.h"

#include <stdarg.h>

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

/* parameter functions. */
al_uri_parameter_t *al_uri_parameter_append (al_uri_t *uri,
   al_uri_parameter_t *prev, const char *name, const char *value);
int al_uri_parameter_free (al_uri_parameter_t *param);
al_uri_parameter_t *al_uri_parameter_get (al_uri_t *uri, const char *name);

/* path functions. */
al_uri_path_t *al_uri_path_append (al_uri_t *uri, al_uri_path_t *prev,
   const char *name);
int al_uri_path_free (al_uri_path_t *path);
al_uri_path_t *al_uri_path_has_v (const al_uri_path_t *path, va_list args);
al_uri_path_t *al_uri_path_has (const al_uri_path_t *path, ...);
al_uri_path_t *al_uri_path_is (const al_uri_path_t *path, ...);
char *al_uri_path_full (const al_uri_path_t *path, char *out, size_t size);
al_uri_path_t *al_uri_path_at (const al_uri_path_t *path, int index);
int al_uri_path_length (const al_uri_path_t *path);

#endif
