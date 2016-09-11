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
   al_flags_t state, flags;
   struct sockaddr_in addr;
   int port, sock_fd, pipe_fd[2];
   fd_set fd_in, fd_out, fd_other;

   /* functions passed to servers. */
   al_server_func *func[AL_SERVER_FUNC_MAX];

   /* connections. */
   al_connection_t *connection_list;

   /* custom data we're passing to the server. */
   al_module_t *module_list;

   /* threading stuff. */
   pthread_t pthread;
   al_mutex_t *mutex;
   int mutex_count;
    
   /* for compatability with C++ wrapper/ */
   void *cpp_wrapper;
};

/* functions for server management. */
al_server_t *al_server_new (int port, al_flags_t flags);
int al_server_set_flags (al_server_t *server, int port, al_flags_t flags);
int al_server_is_open (const al_server_t *server);
int al_server_is_running (const al_server_t *server);
int al_server_is_quitting (const al_server_t *server);
int al_server_is_in_loop (const al_server_t *server);
int al_server_close (al_server_t *server);
int al_server_open (al_server_t *server);
void *al_server_pthread_func (void *arg);
int al_server_start (al_server_t *server);
int al_server_wait (al_server_t *server);
int al_server_run_func (al_server_t *server);
int al_server_stop (al_server_t *server);
int al_server_interrupt (al_server_t *server);
int al_server_free (al_server_t *server);
int al_server_lock (al_server_t *server);
int al_server_unlock (al_server_t *server);
int al_server_func_set (al_server_t *server, int task, al_server_func *func);
int al_server_write (al_server_t *server, const unsigned char *buf,
   size_t size);
int al_server_write_string (al_server_t *server, const char *string);
al_module_t *al_server_module_new (al_server_t *server, const char *name,
   void *data, size_t data_size, al_module_func *free_func);
al_module_t *al_server_module_get (const al_server_t *server,
   const char *name);
int al_server_in_thread (const al_server_t *server);

#endif
