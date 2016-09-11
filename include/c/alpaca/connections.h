/* connections.h
 * -------------
 * connection management for servers. */

#ifndef __ALPACA_C_CONNECTIONS_H
#define __ALPACA_C_CONNECTIONS_H

#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "defs.h"

/* our connections. */
struct _al_connection_t {
   /* flags! */
   al_flags_t flags;

   /* socket stuff. */
   int sock_fd;
   struct sockaddr_in addr;
   socklen_t addr_size;

   /* input/output buffers. */
   unsigned char *input, *output;
   size_t input_size,  input_len,  input_pos;
   size_t output_size, output_len, output_pos, output_max;

   /* custom data assigned to each connection. */
   al_module_t *module_list;

   /* link to server. */
   al_server_t *server;
   al_connection_t *prev, *next;

   /* identifying data. */
   char *ip_address;
};

/* data sent via AL_SERVER_PRE_WRITE_FUNC. */
struct _al_func_pre_write_t {
   unsigned char *data;
   size_t data_len;
};

/* functions for connection management. */
al_connection_t *al_connection_new (al_server_t *server, int fd,
   const struct sockaddr_in *addr, socklen_t addr_size);
int al_connection_free (al_connection_t *c);
int al_connection_close (al_connection_t *c);
int al_connection_append_buffer (al_connection_t *c, unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, const unsigned char *input,
   size_t isize);
int al_connection_fetch_buffer (al_connection_t *c, const unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, unsigned char *output,
   size_t osize);
int al_connection_read (al_connection_t *c, unsigned char *buf, size_t size);
int al_connection_fd_read (al_connection_t *c);
int al_connection_fd_write (al_connection_t *c);
int al_connection_write (al_connection_t *c, const unsigned char *buf,
   size_t size);
int al_connection_write_string (al_connection_t *c, const char *string);
int al_connection_wrote (al_connection_t *c);
int al_connection_stage_output (al_connection_t *c);
al_module_t *al_connection_module_new (al_connection_t *connection,
   const char *name, void *data, size_t data_size, al_module_func *free_func);
al_module_t *al_connection_module_get (const al_connection_t *connection,
   const char *name);

#endif
