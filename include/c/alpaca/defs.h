/* defs.h
 * ------
 * common definitions shared between the client and server. */

#ifndef __ALPACA_C_DEFS_H
#define __ALPACA_C_DEFS_H

#include <stdio.h>

#include "llist.h"
#include "utils.h"

/* default options for HTTP modules. */
#define AL_HTTP_TIMEOUT       5.00f

/* URI flags. */
#define AL_URI_RELATIVE       0x01

/* HTTP versions. */
#define AL_HTTP_INVALID      -1
#define AL_HTTP_0_9           0
#define AL_HTTP_1_0           1
#define AL_HTTP_1_1           2

/* HTTP state flags. */
#define AL_STATE_PERSIST      0x01

/* HTTP states. */
#define AL_STATE_METHOD       0
#define AL_STATE_HEADER       1

/* types of headers. */
#define AL_HEADER_REQUEST     0
#define AL_HEADER_RESPONSE    1

/* connection flags. */
#define AL_CONNECTION_WRITING    0x01
#define AL_CONNECTION_WROTE      0x02
#define AL_CONNECTION_CLOSING    0x04
#define AL_CONNECTION_KEEP_OPEN  0x08
#define AL_CONNECTION_TIMED_OUT  0x10

/* server functions. */
#define AL_SERVER_FUNC_JOIN      0
#define AL_SERVER_FUNC_LEAVE     1
#define AL_SERVER_FUNC_READ      2
#define AL_SERVER_FUNC_PRE_WRITE 3
#define AL_SERVER_FUNC_STOPPED   4
#define AL_SERVER_FUNC_CLOSED    5
#define AL_SERVER_FUNC_TIMEOUT   6
#define AL_SERVER_FUNC_MAX       8

/* server state flags.  unless you're working on server code,
 * these are read-only. */
#define AL_SERVER_STATE_OPEN    0x01
#define AL_SERVER_STATE_QUIT    0x02
#define AL_SERVER_STATE_RUNNING 0x04
#define AL_SERVER_STATE_PIPE    0x08
#define AL_SERVER_STATE_IN_LOOP 0x10

/* server flags. */
/* TODO: 0x01 */
#define AL_SERVER_CLOSE_AFTER_STOP  0x02

/* type definitions. */
typedef unsigned long int al_flags_t;
typedef struct _al_server_t         al_server_t;
typedef struct _al_connection_t     al_connection_t;
typedef struct _al_mutex_t          al_mutex_t;
typedef struct _al_func_read_t      al_func_read_t;
typedef struct _al_func_pre_write_t al_func_pre_write_t;
typedef struct _al_module_t         al_module_t;
typedef struct _al_http_func_def_t  al_http_func_def_t;
typedef struct _al_http_t           al_http_t;
typedef struct _al_http_state_t     al_http_state_t;
typedef struct _al_http_header_t    al_http_header_t;
typedef struct _al_uri_t            al_uri_t;
typedef struct _al_uri_path_t       al_uri_path_t;
typedef struct _al_uri_parameter_t  al_uri_parameter_t;

/* function macros and typedefs. */
#define AL_SERVER_FUNC(x) \
   int x (al_server_t *server, al_connection_t *connection, int func, \
          void *arg)
typedef AL_SERVER_FUNC(al_server_func);

#define AL_MODULE_FUNC(x) \
   int x (al_module_t *module, void *arg)
typedef AL_MODULE_FUNC(al_module_func);

#define AL_HTTP_FUNC(x) \
   int x (al_http_state_t *request, al_http_func_def_t *func, const char *data)
typedef AL_HTTP_FUNC(al_http_func);

#endif
