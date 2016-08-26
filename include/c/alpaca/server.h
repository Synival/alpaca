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
struct _al_server_t {
   /* internal stuff. */
   al_flags_t flags;
   struct sockaddr_in addr;
   int port, sock_fd, pipe_fd[2];
   fd_set fd_in, fd_out, fd_other;

   /* functions passed to servers. */
   al_server_func *func[AL_SERVER_FUNC_MAX];

   /* connections. */
   al_connection_t *connection_list;

   /* custom data we're passing to the server. */
   void *data;
   size_t data_size;

   /* threading stuff. */
   pthread_t pthread;
   al_mutex_t *mutex;
   int mutex_count;
};

/* functions for server management. */
al_server_t *al_server_new (int port, al_flags_t flags);
int al_server_is_open (al_server_t *server);
int al_server_is_running (al_server_t *server);
int al_server_close (al_server_t *server);
int al_server_open (al_server_t *server);
int al_server_add_fd (al_server_t *server, int fd);
void *al_server_pthread_func (void *arg);
int al_server_start (al_server_t *server);
int al_server_wait (al_server_t *server);
int al_server_run_func (al_server_t *server);
int al_server_new_connection (al_server_t *server, int fd,
   struct sockaddr_in *addr, socklen_t addr_size);
int al_server_stop (al_server_t *server);
int al_server_interrupt (al_server_t *server);
int al_server_free (al_server_t *server);
int al_server_lock (al_server_t *server);
int al_server_unlock (al_server_t *server);
int al_server_func_read (al_server_t *server, al_server_func *func);
int al_server_func_join (al_server_t *server, al_server_func *func);
int al_server_func_leave (al_server_t *server, al_server_func *func);
int al_server_func_set (al_server_t *server, int ref, al_server_func *func);

#endif
