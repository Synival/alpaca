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
};

/* state information for each connection. */
struct _al_http_state_t {
   int state, version;
   al_flags_t flags;
   char *verb, *uri, *version_str;
   al_connection_t *connection;
};

/* functions. */
al_http_t *al_http_init (al_server_t *server);
al_http_t *al_http_get (al_server_t *server);
al_http_state_t *al_http_get_state (al_connection_t *connection);
al_http_func_def_t *al_http_set_func (al_http_t *http, char *verb,
   al_http_func *func);
al_http_func_def_t *al_http_get_func (al_http_t *http, char *verb);
int al_http_free_func (al_http_func_def_t *rf);
int al_http_state_method (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state, char *line);
int al_http_state_header (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state, char *line);
int al_http_state_finish (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state);
int al_http_write_string (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state, char *string);
int al_http_state_reset (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state);
int al_http_state_cleanup (al_http_state_t *state);

/* hooks and default functions. */
AL_MODULE_FUNC (al_http_data_free);
AL_MODULE_FUNC (al_http_state_data_free);
AL_SERVER_FUNC (al_http_func_read);
AL_SERVER_FUNC (al_http_func_join);
AL_SERVER_FUNC (al_http_func_leave);

#endif
