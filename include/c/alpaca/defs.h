/* defs.h
 * ------
 * common definitions shared between the client and server. */

#ifndef __ALPACA_C_DEFS_H
#define __ALPACA_C_DEFS_H

#include <stdio.h>

#include "llist.h"
#include "utils.h"

/* connection flags. */
#define AL_CONNECTION_WRITING 0x01
#define AL_CONNECTION_WROTE   0x02

/* server functions. */
#define AL_SERVER_FUNC_JOIN      0
#define AL_SERVER_FUNC_LEAVE     1
#define AL_SERVER_FUNC_READ      2
#define AL_SERVER_FUNC_PRE_WRITE 3
#define AL_SERVER_FUNC_MAX       4

/* server flags. */
#define AL_SERVER_OPEN    0x01
#define AL_SERVER_QUIT    0x02
#define AL_SERVER_RUNNING 0x04
#define AL_SERVER_PIPE    0x08

/* type definitions. */
typedef unsigned long int al_flags_t;
typedef struct _al_server_t     al_server_t;
typedef struct _al_connection_t al_connection_t;
typedef struct _al_mutex_t      al_mutex_t;

/* function macros and typedefs. */
#define AL_SERVER_FUNC(x) \
   int x (al_server_t *server, al_connection_t *connection, \
          void *data, size_t data_size)
typedef AL_SERVER_FUNC(al_server_func);

#endif
