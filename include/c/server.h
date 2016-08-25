/* server.h
 * --------
 * low-level server functions for AlPACA. */

#ifndef __ALPACA_C_SERVER_H
#define __ALPACA_C_SERVER_H

#include <pthread.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "defs.h"

#ifndef _WIN32
   #define SOCKET_ERROR -1
   #define SOCKET_ERRNO (errno)
   #define socket_close close
#else
   #define SOCKET_ERRNO (WSAGetLastError ())
   #define socket_close closesocket
#endif

/* structure for servers. */
struct _server_type {
   /* internal stuff. */
   flags_type flags;
   struct sockaddr_in addr;
   int port, sock_fd, pipe_fd[2];
   fd_set fd_in, fd_out, fd_other;

   /* functions passed to servers. */
   server_func *func[SERVER_FUNC_MAX];

   /* connections. */
   connection_type *connection_list;

   /* custom data we're passing to the server. */
   void *data;
   size_t data_size;

   /* threading stuff. */
   pthread_t pthread;
   pthread_mutex_t mutex;
   int mutex_count;
};

/* functions for server management. */
server_type *server_new (int port, flags_type flags);
int server_is_open (server_type *server);
int server_is_running (server_type *server);
int server_close (server_type *server);
int server_open (server_type *server);
int server_add_fd (server_type *server, int fd);
void *server_pthread_func (void *arg);
int server_start (server_type *server);
int server_wait (server_type *server);
int server_run_func (server_type *server);
int server_new_connection (server_type *server, int fd,
                           struct sockaddr_in *addr, socklen_t addr_size);
int server_stop (server_type *server);
int server_interrupt (server_type *server);
int server_free (server_type *server);
int server_lock (server_type *server);
int server_unlock (server_type *server);
int server_func_read (server_type *server, server_func *func);
int server_func_join (server_type *server, server_func *func);
int server_func_leave (server_type *server, server_func *func);
int server_func_set (server_type *server, int ref, server_func *func);

#endif
