/* server.c
 * --------
 * low-level server functions for AlPACA. */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "alpaca/connections.h"

#include "alpaca/server.h"

server_type *server_new (int port, flags_type flags)
{
   server_type *new;
   pthread_mutexattr_t p_attr;

   /* create an empty structure. */
   new = malloc (sizeof (server_type));
   memset (new, 0, sizeof (server_type));
   new->port = port;

   /* create a mutex for our running thread. */
   pthread_mutexattr_init (&p_attr);
   pthread_mutexattr_settype (&p_attr, PTHREAD_MUTEX_RECURSIVE);
   pthread_mutex_init (&(new->mutex), &p_attr);

   /* we did it! */
   return new;
}

int server_is_open (server_type *server)
   { return (server->flags & SERVER_OPEN) ? 1 : 0; }
int server_is_running (server_type *server)
   { return (server->flags & SERVER_RUNNING) ? 1 : 0; }

int server_lock (server_type *server)
{
   if (pthread_mutex_lock (&(server->mutex)) != 0)
      return 0;
   server->mutex_count++;
   return 1;
}
int server_unlock (server_type *server)
{
   if (pthread_mutex_unlock (&(server->mutex)) != 0)
      return 0;
   server->mutex_count--;
   return 1;
}

int server_close (server_type *server)
{
   /* don't do anything if the server is currently closed. */
   if (!server_is_open (server))
      return 0;

   /* server can't be running anymore. */
   if (server_is_running (server)) {
      server_stop (server);
      server_wait (server);
   }

   /* lock server while we're doing this stuff. */
   server_lock (server);

   /* close connections. */
   while (server->connection_list)
      connection_free (server->connection_list);

   /* close socket. */
   socket_close (server->sock_fd);

   /* close pipe. */
   if (server->flags & SERVER_PIPE) {
      server->flags &= ~SERVER_PIPE;
      close (server->pipe_fd[0]);
      close (server->pipe_fd[1]);
   }

   /* remove SERVER_OPEN flag and return success. */
   server->flags &= ~SERVER_OPEN;
   server_unlock (server);
   return 1;
}

int server_open (server_type *server)
{
   int fd, flags, i, optval;

   /* don't do anything if the server is currently open. */
   if (server_is_open (server))
      return 0;

   /* attempt to get a TCP/IP socket. */
   if ((fd = socket (AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
      ERROR ("Unable to open socket (Error %d).\n", SOCKET_ERRNO);
      return 0;
   }

   /* let this be reusable. */
   optval = 1;
   setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

   /* set up parameters for bind(). */
   memset (&(server->addr), 0, sizeof (struct sockaddr_in));
   server->addr.sin_family      = AF_INET;
   server->addr.sin_addr.s_addr = INADDR_ANY;
   server->addr.sin_port        = htons (server->port);

   /* bind the socket to a port on the server. */
   if (bind (fd, (struct sockaddr *) &(server->addr),
             sizeof (struct sockaddr_in)) != 0) {
      ERROR ("Unable to bind TCP/IP socket to port %d (Error %d).\n",
             server->port, SOCKET_ERRNO);
      socket_close (fd);
      return 0;
   }

   /* listen for new connections. */
   if (listen (fd, 5) < 0) {
      ERROR ("Unable to listen() on port %d (Error %d).\n",
             server->port, SOCKET_ERRNO);
      socket_close (fd);
      return 0;
   }

   /* attempt to create a pipe we can use for select() interrupts. */
   memset (server->pipe_fd, 0, sizeof (int) * 2);
   if (pipe (server->pipe_fd) != 0) {
      ERROR ("Warning: Unable to create pipe (Error %d). \n"
             "Continuing anyway.\n", errno);
   }
   else {
      /* make both ends non-blocking, just to be safe. */
      for (i = 0; i < 2; i++) {
         flags = fcntl (server->pipe_fd[i], F_GETFL);
         flags |= O_NONBLOCK;
         fcntl (server->pipe_fd[i], F_SETFL, flags);
      }

      /* remember that we have a pipe. */
      server->flags |= SERVER_PIPE;
   }

   /* set flags for our server. */
   server->flags |= SERVER_OPEN;
   server->sock_fd = fd;

   /* return success. */
   return 1;
}

int server_run_func (server_type *server)
{
   connection_type *c, *c_next;
   struct sockaddr_in client_addr;
   socklen_t client_addr_size;
   int fd, fd_max, res, used;

   /* before we wait, make sure our data is sane. */
   server_lock (server);

   /* some silly preparations for accept(). */
   client_addr_size = sizeof (struct sockaddr_in);

   /* zero-out our descriptor sets for select(). */
   FD_ZERO (&(server->fd_in));
   FD_ZERO (&(server->fd_out));
   FD_ZERO (&(server->fd_other));

   /* add our listening server. */
   FD_SET (server->sock_fd, &(server->fd_in));
   fd_max = server->sock_fd;

   /* do we have a pipe?  read from it. */
   if (server->flags & SERVER_PIPE) {
      FD_SET (server->pipe_fd[0], &(server->fd_in));
      fd_max = MAX (fd_max, server->pipe_fd[0]);
   }

   /* add all other connections. */
   for (c = server->connection_list; c != NULL; c = c->next) {
      FD_SET (c->sock_fd, &(server->fd_in));
      FD_SET (c->sock_fd, &(server->fd_other));
      fd_max  = MAX (fd_max, c->sock_fd);

      /* if there's stuff to write, add to the write buffer. */
      if (c->flags & CONNECTION_WROTE) {
         if (!(c->flags & CONNECTION_WRITING)) {
            if (server->func[SERVER_FUNC_PRE_WRITE])
               server->func[SERVER_FUNC_PRE_WRITE] (server, c,
                  c->output, c->output_len);
            c->output_max = c->output_len - c->output_pos;
            c->flags |= CONNECTION_WRITING;
         }
         FD_SET (c->sock_fd, &(server->fd_out));
      }
   }

   /* don't greedily lock the server while select() is waiting. */
   server_unlock (server);

   /* wait forever until we have some activity. */
   if ((res = select (fd_max + 1, &(server->fd_in), &(server->fd_out),
               &(server->fd_other), NULL)) == SOCKET_ERROR) {
      if (errno != EINTR)
         ERROR ("select() error: %d\n", errno);
      server->flags |= SERVER_QUIT;
      return 0;
   }

   /* lock our server. */
   server_lock (server);

   /* clear out data from our pipe. */
   if (server->flags & SERVER_PIPE)
      if (FD_ISSET (server->pipe_fd[0], &(server->fd_in))) {
         unsigned char buf[256];
         res = read (server->pipe_fd[0], buf, 256);
      }

   /* check for incoming connections. */
   if (FD_ISSET (server->sock_fd, &(server->fd_in))) {
      memset (&client_addr, 0, sizeof (struct sockaddr_in));
      if ((fd = accept (server->sock_fd, (struct sockaddr *) &client_addr,
                        &client_addr_size)) < 0)
         ERROR ("accept() error: %d\n", errno);
      else
         connection_new (server, fd, &client_addr, client_addr_size);
   }

   /* check all of our connections. */
   for (c = server->connection_list; c != NULL; c = c_next) {
      c_next = c->next;

      /* close connections with errors. */
      if (FD_ISSET (c->sock_fd, &(server->fd_other))) {
         connection_free (c);
         continue;
      }

      /* can we input? */
      if (FD_ISSET (c->sock_fd, &(server->fd_in))) {
         if (connection_fd_read (c) < 0) {
            connection_free (c);
            continue;
         }
         while (c->input_len > c->input_pos && server->func[SERVER_FUNC_READ]) {
            used = server->func[SERVER_FUNC_READ] (server, c,
               c->input + c->input_pos, c->input_len - c->input_pos);
            if (used >= c->input_len - c->input_pos) {
               c->input_len = 0;
               c->input_pos = 0;
            }
            else if (used >= 1)
               c->input_pos += used;
            else
               break;
         }
      }

      /* can we output? */
      if (FD_ISSET (c->sock_fd, &(server->fd_out)))
         if (connection_fd_write (c) < 0) {
            connection_free (c);
            continue;
         }
   }

   /* unlock server and return success. */
   server_unlock (server);
   return 1;
}

void *server_pthread_func (void *arg)
{
   server_type *server;
   server = arg;

   /* run until the server is told to quit. */
   while (!(server->flags & SERVER_QUIT))
      server_run_func (server);

   /* close our server. */
   server_close (server);

   /* mark that we're no longer running and return success. */
   server->flags &= ~SERVER_RUNNING;
   return NULL;
}

int server_start (server_type *server)
{
   int res;

   /* don't do anything if the server is already running. */
   if (server_is_running (server))
      return 0;

   /* don't do anything if we couldn't open the server. */
   if (!server_is_open (server))
      if (!server_open (server))
         return 0;

   /* attempt to start a pthread. */
   server->flags |= SERVER_RUNNING;
   if ((res = pthread_create (&(server->pthread), NULL, server_pthread_func,
                              (void *) server)) != 0) {
      server->flags &= ~SERVER_RUNNING;
      ERROR ("Unable to start server (Error: %d)\n", res);
      return 0;
   }

   /* return success. */
   return 1;
}

int server_wait (server_type *server)
{
   if (!server_is_running (server))
      return 0;
   pthread_join (server->pthread, NULL);
   return 1;
}

int server_interrupt (server_type *server)
{
   /* must be running. */
   if (!server_is_running (server))
      return 0;

   /* must have a pipe we can send something to. */
   if (!(server->flags & SERVER_PIPE))
      return 0;

   /* write something! */
   if (write (server->pipe_fd[1], "\x01", 1) != 1) {
      PRINTF ("server_interrupt() failed.\n");
      return 0;
   }

   return 1;
}

int server_stop (server_type *server)
{
   if (!server_is_running (server))
      return 0;
   if (server->flags & SERVER_QUIT)
      return 0;
   server->flags |= SERVER_QUIT;
   server_interrupt (server);
   return 1;
}

int server_free (server_type *server)
{
   /* stop our server. */
   if (server_is_running (server)) {
      server_stop (server);
      server_wait (server);
   }

   /* make sure it's closed, just in case. */
   if (server_is_open (server))
      server_close (server);

   /* destroy our mutex. */
   pthread_mutex_destroy (&(server->mutex));

   free (server);
   return 1;
}

int server_func_set (server_type *server, int ref, server_func *func)
{
   /* error-checking. */
   if (server == NULL)
      return 0;
   if (ref < 0 || ref >= SERVER_FUNC_MAX)
      return 0;

   /* safely set function and return success. */
   server_lock (server);
   server->func[ref] = func;
   server_unlock (server);
   return 1;
}
