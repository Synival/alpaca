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
struct _connection_type {
   /* flags! */
   flags_type flags;

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
   server_type *server;
   connection_type *prev, *next;
};

/* functions for connection management. */
connection_type *connection_new (server_type *server, int fd,
   struct sockaddr_in *addr, socklen_t addr_size);
int connection_free (connection_type *c);
int connection_append_buffer (connection_type *c, unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, unsigned char *input, size_t isize);
int connection_fetch_buffer (connection_type *c, unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, unsigned char *output, size_t osize);
int connection_read (connection_type *c, unsigned char *buf, size_t size);
int connection_fd_read (connection_type *c);
int connection_fd_write (connection_type *c);
int connection_write (connection_type *c, unsigned char *buf, size_t size);
int connection_write_all (server_type *server, unsigned char *buf, size_t size);
int connection_wrote (connection_type *c);

#endif
