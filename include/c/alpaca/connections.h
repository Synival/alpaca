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
   void *data;

   /* link to server. */
   al_server_t *server;
   al_connection_t *prev, *next;
};

/* functions for connection management. */
al_connection_t *al_connection_new (al_server_t *server, int fd,
   struct sockaddr_in *addr, socklen_t addr_size);
int al_connection_free (al_connection_t *c);
int al_connection_append_buffer (al_connection_t *c, unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, unsigned char *input, size_t isize);
int al_connection_fetch_buffer (al_connection_t *c, unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, unsigned char *output, size_t osize);
int al_connection_read (al_connection_t *c, unsigned char *buf, size_t size);
int al_connection_fd_read (al_connection_t *c);
int al_connection_fd_write (al_connection_t *c);
int al_connection_write (al_connection_t *c, unsigned char *buf,
   size_t size);
int al_connection_write_all (al_server_t *server, unsigned char *buf,
   size_t size);
int al_connection_wrote (al_connection_t *c);

#endif