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
#include "alpaca/mutex.h"

#include "alpaca/server.h"

al_server_t *al_server_new (int port, al_flags_t flags)
{
   al_server_t *new;

   /* create an empty structure. */
   new = calloc (1, sizeof (al_server_t));
   new->port = port;

   /* create a mutex for our running thread. */
   new->mutex = al_mutex_new ();

   /* we did it! */
   return new;
}

int al_server_is_open (al_server_t *server)
   { return (server->flags & AL_SERVER_OPEN) ? 1 : 0; }
int al_server_is_running (al_server_t *server)
   { return (server->flags & AL_SERVER_RUNNING) ? 1 : 0; }

int al_server_lock (al_server_t *server)
{
   if (al_mutex_lock (server->mutex) != 0)
      return 0;
   server->mutex_count++;
   return 1;
}
int al_server_unlock (al_server_t *server)
{
   if (al_mutex_unlock (server->mutex) != 0)
      return 0;
   server->mutex_count--;
   return 1;
}

int al_server_close (al_server_t *server)
{
   /* don't do anything if the server is currently closed. */
   if (!al_server_is_open (server))
      return 0;

   /* server can't be running anymore. */
   if (al_server_is_running (server)) {
      al_server_stop (server);
      al_server_wait (server);
   }

   /* lock server while we're doing this stuff. */
   al_server_lock (server);

   /* close connections. */
   while (server->connection_list)
      al_connection_free (server->connection_list);

   /* close socket. */
   socket_close (server->sock_fd);

   /* close pipe. */
   if (server->flags & AL_SERVER_PIPE) {
      server->flags &= ~AL_SERVER_PIPE;
      close (server->pipe_fd[0]);
      close (server->pipe_fd[1]);
   }

   /* remove AL_SERVER_OPEN flag and return success. */
   server->flags &= ~AL_SERVER_OPEN;
   al_server_unlock (server);
   return 1;
}

int al_server_open (al_server_t *server)
{
   int fd, flags, i, optval;

   /* don't do anything if the server is currently open. */
   if (al_server_is_open (server))
      return 0;

   /* attempt to get a TCP/IP socket. */
   if ((fd = socket (AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
      AL_ERROR ("Unable to open socket (Error %d).\n", SOCKET_ERRNO);
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
      AL_ERROR ("Unable to bind TCP/IP socket to port %d (Error %d).\n",
                server->port, SOCKET_ERRNO);
      socket_close (fd);
      return 0;
   }

   /* listen for new connections. */
   if (listen (fd, 5) < 0) {
      AL_ERROR ("Unable to listen() on port %d (Error %d).\n",
                server->port, SOCKET_ERRNO);
      socket_close (fd);
      return 0;
   }

   /* attempt to create a pipe we can use for select() interrupts. */
   memset (server->pipe_fd, 0, sizeof (int) * 2);
   if (pipe (server->pipe_fd) != 0) {
      AL_ERROR ("Warning: Unable to create pipe (Error %d). \n"
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
      server->flags |= AL_SERVER_PIPE;
   }

   /* set flags for our server. */
   server->flags |= AL_SERVER_OPEN;
   server->sock_fd = fd;

   /* return success. */
   return 1;
}

int al_server_run_func (al_server_t *server)
{
   al_connection_t *c, *c_next;
   struct sockaddr_in client_addr;
   socklen_t client_addr_size;
   int fd, fd_max, res, bytes_read;

   /* before we wait, make sure our data is sane. */
   al_server_lock (server);

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
   if (server->flags & AL_SERVER_PIPE) {
      FD_SET (server->pipe_fd[0], &(server->fd_in));
      fd_max = AL_MAX (fd_max, server->pipe_fd[0]);
   }

   /* add all other connections. */
   for (c = server->connection_list; c != NULL; c = c->next) {
      FD_SET (c->sock_fd, &(server->fd_in));
      FD_SET (c->sock_fd, &(server->fd_other));
      fd_max = AL_MAX (fd_max, c->sock_fd);

      /* if there's stuff to write, add to the write buffer. */
      al_connection_stage_output (c);
      if (c->flags & AL_CONNECTION_WRITING)
         FD_SET (c->sock_fd, &(server->fd_out));
   }

   /* don't greedily lock the server while select() is waiting. */
   al_server_unlock (server);

   /* wait forever until we have some activity. */
   if ((res = select (fd_max + 1, &(server->fd_in), &(server->fd_out),
               &(server->fd_other), NULL)) == SOCKET_ERROR) {
      if (errno != EINTR)
         AL_ERROR ("select() error: %d\n", errno);
      server->flags |= AL_SERVER_QUIT;
      return 0;
   }

   /* lock our server. */
   al_server_lock (server);

   /* clear out data from our pipe. */
   if (server->flags & AL_SERVER_PIPE)
      if (FD_ISSET (server->pipe_fd[0], &(server->fd_in))) {
         unsigned char buf[256];
         res = read (server->pipe_fd[0], buf, 256);
      }

   /* check for incoming connections. */
   if (FD_ISSET (server->sock_fd, &(server->fd_in))) {
      memset (&client_addr, 0, sizeof (struct sockaddr_in));
      if ((fd = accept (server->sock_fd, (struct sockaddr *) &client_addr,
                        &client_addr_size)) < 0)
         AL_ERROR ("accept() error: %d\n", errno);
      else
         al_connection_new (server, fd, &client_addr, client_addr_size);
   }

   /* check all of our connections for input. */
   for (c = server->connection_list; c != NULL; c = c_next) {
      c_next = c->next;

      /* close connections with errors. */
      if (FD_ISSET (c->sock_fd, &(server->fd_other))) {
         al_connection_free (c);
         continue;
      }

      /* can we input? */
      if (FD_ISSET (c->sock_fd, &(server->fd_in))) {
         if ((bytes_read = al_connection_fd_read (c)) < 0) {
            al_connection_free (c);
            continue;
         }
         while (c->input_len > c->input_pos &&
                server->func[AL_SERVER_FUNC_READ]) {
            al_func_read_t data = {
               .data         = c->input + c->input_pos,
               .data_len     = c->input_len - c->input_pos,
               .new_data     = c->input + c->input_len - bytes_read,
               .new_data_len = bytes_read,
               .bytes_used   = 0
            };
            server->func[AL_SERVER_FUNC_READ] (server, c,
               AL_SERVER_FUNC_READ, &data);
            if (data.bytes_used >= c->input_len - c->input_pos) {
               c->input_len = 0;
               c->input_pos = 0;
            }
            else if (data.bytes_used >= 1) {
               c->input_pos += data.bytes_used;
               bytes_read    = data.new_data_len;
            }
            else
               break;
         }
      }
   }

   /* check all of our connections for output. */
   for (c = server->connection_list; c != NULL; c = c_next) {
      c_next = c->next;
      if (FD_ISSET (c->sock_fd, &(server->fd_out)))
         if (al_connection_fd_write (c) < 0) {
            al_connection_free (c);
            continue;
         }
   }

   /* unlock server and return success. */
   al_server_unlock (server);
   return 1;
}

void *al_server_pthread_func (void *arg)
{
   al_server_t *server;
   server = arg;

   /* run until the server is told to quit. */
   while (!(server->flags & AL_SERVER_QUIT))
      al_server_run_func (server);

   /* close our server. */
   al_server_close (server);

   /* mark that we're no longer running and return success. */
   server->flags &= ~AL_SERVER_RUNNING;
   return NULL;
}

int al_server_start (al_server_t *server)
{
   int res;

   /* don't do anything if the server is already running. */
   if (al_server_is_running (server))
      return 0;

   /* don't do anything if we couldn't open the server. */
   if (!al_server_is_open (server))
      if (!al_server_open (server))
         return 0;

   /* attempt to start a pthread. */
   server->flags |= AL_SERVER_RUNNING;
   if ((res = pthread_create (&(server->pthread), NULL, al_server_pthread_func,
                              (void *) server)) != 0) {
      server->flags &= ~AL_SERVER_RUNNING;
      AL_ERROR ("Unable to start server (Error: %d)\n", res);
      return 0;
   }

   /* return success. */
   return 1;
}

int al_server_wait (al_server_t *server)
{
   if (!al_server_is_running (server))
      return 0;
   pthread_join (server->pthread, NULL);
   return 1;
}

int al_server_interrupt (al_server_t *server)
{
   /* must be running. */
   if (!al_server_is_running (server))
      return 0;

   /* must have a pipe we can send something to. */
   if (!(server->flags & AL_SERVER_PIPE))
      return 0;

   /* write something! */
   if (write (server->pipe_fd[1], "\x01", 1) != 1) {
      AL_PRINTF ("server_interrupt() failed.\n");
      return 0;
   }

   return 1;
}

int al_server_stop (al_server_t *server)
{
   if (!al_server_is_running (server))
      return 0;
   if (server->flags & AL_SERVER_QUIT)
      return 0;
   server->flags |= AL_SERVER_QUIT;
   al_server_interrupt (server);
   return 1;
}

int al_server_free (al_server_t *server)
{
   /* stop our server. */
   if (al_server_is_running (server)) {
      al_server_stop (server);
      al_server_wait (server);
   }

   /* make sure it's closed, just in case. */
   if (al_server_is_open (server))
      al_server_close (server);

   /* destroy our mutex. */
   if (server->mutex)
      al_mutex_free (server->mutex);

   free (server);
   return 1;
}

int al_server_func_set (al_server_t *server, int ref, al_server_func *func)
{
   /* error-checking. */
   if (server == NULL)
      return 0;
   if (ref < 0 || ref >= AL_SERVER_FUNC_MAX)
      return 0;

   /* safely set function and return success. */
   al_server_lock (server);
   server->func[ref] = func;
   al_server_unlock (server);
   return 1;
}

int al_server_write (al_server_t *server, unsigned char *buf, size_t size)
{
   al_connection_t *c;
   int count;

   /* connection_write() to everyone! */
   al_server_lock (server);
   count = 0;
   for (c = server->connection_list; c != NULL; c = c->next)
      count += al_connection_write (c, buf, size);
   al_server_unlock (server);

   /* return the number of connections written to. */
   return count;
}

int al_server_write_string (al_server_t *server, char *string)
{
   return al_server_write (server, (unsigned char *) string, strlen (string));
}
