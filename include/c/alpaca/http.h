/* http.h
 * ------
 * HTTP API development tools. */

#ifndef __ALPACA_C_HTTP_H
#define __ALPACA_C_HTTP_H

#include "defs.h"

/* definitions for http functions. */
struct _al_http_func_def_t {
   char *verb;
   al_http_t *http;
   al_http_func *func;
   al_http_func_def_t *prev, *next;
};

/* data for our http module. */
struct _al_http_t {
   al_server_t *server;
   al_module_t *module;
   al_http_func_def_t *func_list;

   /* default options. */
   float timeout;
};

/* state information for each connection. */
struct _al_http_state_t {
   int state, version, status_code;
   al_flags_t flags;
   char *verb, *uri_str, *version_str;
   al_connection_t *connection;
   al_http_t *http;
   al_http_header_t *header_list;
   al_uri_t *uri;

   /* output buffer. */
   unsigned char *output;
   size_t output_size, output_len, output_pos;
};

/* header information. */
struct _al_http_header_t {
   char *name, *value;
   al_http_state_t *state;
   al_http_header_t *prev, *next;
};

/* top-level http mangement functions. */
al_http_t *al_http_init (al_server_t *server);
al_http_t *al_http_get (const al_server_t *server);
al_http_state_t *al_http_get_state (const al_connection_t *connection);
al_http_func_def_t *al_http_set_func (al_http_t *http, const char *verb,
   al_http_func *func);
al_http_func_def_t *al_http_get_func (const al_http_t *http, const char *verb);
int al_http_free_func (al_http_func_def_t *rf);

/* state management. */
int al_http_state_method  (al_http_state_t *state, const char *line);
int al_http_state_header  (al_http_state_t *state, const char *line);
int al_http_state_finish  (al_http_state_t *state);
int al_http_state_reset   (al_http_state_t *state);
int al_http_state_cleanup (al_http_state_t *state);
int al_http_state_cleanup_output (al_http_state_t *state);

/* state header management. */
al_http_header_t *al_http_header_set (al_http_state_t *state,
   const char *name, const char *value);
al_http_header_t *al_http_header_get (const al_http_state_t *state,
   const char *name);
int al_http_header_free (al_http_header_t *h);
int al_http_header_clear (al_http_state_t *state);
const char *al_http_status_code_string (int status_code);
int al_http_set_status_code (al_http_state_t *state, int status_code);

/* writing to clients. */
int al_http_write (al_http_state_t *state, const unsigned char *buf,
   size_t size);
int al_http_write_string (al_http_state_t *state, const char *string);
int al_http_write_finish (al_http_state_t *state);

/* hooks and default functions. */
AL_MODULE_FUNC (al_http_data_free);
AL_MODULE_FUNC (al_http_state_data_free);
AL_SERVER_FUNC (al_http_func_read);
AL_SERVER_FUNC (al_http_func_join);
AL_SERVER_FUNC (al_http_func_leave);

#endif
