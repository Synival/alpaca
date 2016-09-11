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
#include "alpaca/modules.h"
#include "alpaca/mutex.h"

#include "alpaca/server.h"

/* al_server_new():
 * ----------------
 * Creates a new server instance.  This is the top-level structure for all
 * AlPACA routines and should generally be called first.
 *
 * port:  Port used for listening (ex: 80 for HTTP)
 * flags: Optional bit flags to enable certain features or modify behavior.
 *        (see al_server_set_flags() for details
 *
 * Return value: A pointer to a new server instance or NULL on failure.
 */
al_server_t *al_server_new (int port, al_flags_t flags)
{
   al_server_t *new;

   /* create an empty structure with a mutex. */
   new = calloc (1, sizeof (al_server_t));
   new->port = port;

   /* create a mutex for our running thread. */
   new->mutex = al_mutex_new ();

   /* set port + flags. */
   al_server_set_flags (new, port, flags);

   /* we did it! */
   return new;
}

/* al_server_set_flags():
 * ----------------------
 * Sets the port and flags of the server before the connection is opened.
 * Necessary if the server owner wants to shut down the server, change
 * settings, and start it up again.
 *
 * server: Server whose flags are being modified
 * port:   Port used for listening (ex: 80 for HTTP)
 * flags:  Optional bit flags to enable certain features or modify behavior.
 *    (no flags yet)
 */
int al_server_set_flags (al_server_t *server, int port, al_flags_t flags)
{
   /* don't allow port + flags to be set if the server is currently open. */
   al_server_lock (server);
   if (al_server_is_open (server)) {
      al_server_unlock (server);
      return 0;
   }

   /* server is closed; set port + flags and return success. */
   server->port  = port;
   server->flags = flags;
   al_server_unlock (server);
   return 1;
}

/* al_server_is_open():      (checks AL_SERVER_STATE_OPEN)
 * al_server_is_running():   (checks AL_SERVER_STATE_RUNNING)
 * al_server_is_quitting():  (checks AL_SERVER_STATE_QUITTING)
 * al_server_is_in_loop():   (checks AL_SERVER_STATE_IN_LOOP)
 * -------------------------------------------------------------
 * Return value: 1 if the flag checked is on, otherwise 0.
 */
int al_server_is_open (const al_server_t *server)
   { return (server->state & AL_SERVER_STATE_OPEN) ? 1 : 0; }
int al_server_is_running (const al_server_t *server)
   { return (server->state & AL_SERVER_STATE_RUNNING) ? 1 : 0; }
int al_server_is_quitting (const al_server_t *server)
   { return (server->state & AL_SERVER_STATE_QUIT) ? 1 : 0; }
int al_server_is_in_loop (const al_server_t *server)
   { return (server->state & AL_SERVER_STATE_IN_LOOP) ? 1 : 0; }

/* al_server_lock():
 * -----------------
 * Claims ownership of server/connection resources.  Can be used recursively.
 *
 * Return value: 1 on success, 0 on failure.
 */
int al_server_lock (al_server_t *server)
{
   /* lock the mutex and return 0 if there was an error. */
   if (al_mutex_lock (server->mutex) != 0)
      return 0;
   server->mutex_count++;
   return 1;
}

/* al_server_unlock():
 * -------------------
 * Relinquishes ownership of server/connection resources.  If al_server_lock()
 * has been called multiple times, this function must be called an equal number
 * of times for the thread's mutex to be unlocked.
 *
 * Return value: 1 on success, 0 on failure.
 */
int al_server_unlock (al_server_t *server)
{
   /* unlock the mutex and return 0 if there was an error. */
   if (al_mutex_unlock (server->mutex) != 0)
      return 0;
   server->mutex_count--;
   return 1;
}

/* al_server_close():
 * ------------------
 * Closes all the server's connections, the pipe, and the listening socket.
 * If the thread for the server loop is running, it is closed before
 * proceeding.  This function cannot be called from within the server loop.
 *
 * Returns 1 if all sockets have been closed, 0 if the server wasn't open.
 */
int al_server_close (al_server_t *server)
{
   /* although this can be run from the *thread*, it can't be run from
    * inside the server *loop*, which relies on the server being open. */
   if (al_server_is_in_loop (server)) {
      AL_ERROR ("al_server_close() called from within server loop!\n");
      return 0;
   }

   /* don't do anything if the server is currently closed. */
   if (!al_server_is_open (server))
      return 0;

   /* the server shouldn't be running, but if it is, close it and wait
    * patiently for the thread to die. */
   if (al_server_is_running (server)) {
      al_server_stop (server);
      al_server_wait (server);
   }

   /* lock the server while we're manipulating its state. */
   al_server_lock (server);

   /* close connections. */
   while (server->connection_list)
      al_connection_free (server->connection_list);

   /* close socket. */
   socket_close (server->sock_fd);

   /* close our pipe. */
   if (server->state & AL_SERVER_STATE_PIPE) {
      server->state &= ~AL_SERVER_STATE_PIPE;
      close (server->pipe_fd[0]);
      close (server->pipe_fd[1]);
   }

   /* indicate that the server is no longer open. */
   server->state &= ~AL_SERVER_STATE_OPEN;

   /* relinquish control and return success. */
   al_server_unlock (server);
   return 1;
}

/* al_server_open():
 * -----------------
 * Opens a port for listening and accepting incoming connections.  This
 * function also creates a pipe used for interrupting the thread containing
 * the server loop.
 *
 * Return value: 1 on success, 0 on failure of any kind.
 */
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

   /* let this be reusable.  this lets the server reclaim control of this
    * port on the event of a crash and there were connections that did not
    * close cleanly. */
   /* TODO: make sure this isn't a security risk or something. */
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

   /* attempt to create a pipe we can use for select() interrupts.
    * we use this pipe to "wake up" the server thread for events like
    * shutting down, forcing output to be queued, and anything else that
    * needs select() to stop waiting. */
   memset (server->pipe_fd, 0, sizeof (int) * 2);
   if (pipe (server->pipe_fd) != 0) {
      AL_ERROR ("Warning: Unable to create pipe (Error %d). \n"
                "Continuing anyway.\n", errno);
   }
   /* pipe() worked - set some things up. */
   else {
      /* make both ends of the pipe non-blocking so the pipe is never
       * something being waited upon. */
      for (i = 0; i < 2; i++) {
         flags = fcntl (server->pipe_fd[i], F_GETFL);
         flags |= O_NONBLOCK;
         fcntl (server->pipe_fd[i], F_SETFL, flags);
      }

      /* remember that we have a pipe. */
      server->state |= AL_SERVER_STATE_PIPE;
   }

   /* record our listening socket and mark that our server is now open. */
   server->sock_fd = fd;
   server->state |= AL_SERVER_STATE_OPEN;

   /* return success. */
   return 1;
}

/* al_server_loop_func():
 * ----------------------
 * This function is called from the server loop in al_server_pthread_func().
 * It does several important things:
 *
 *    1) Stage data to be sent out to connections,
 *    2) Use select() to wait until connections are ready for I/O,
 *    3) Read from connections with pending input and run function hooks,
 *    4) Write staged output to connections ready for output.
 *
 * Return value: 1 on success, 0 on failure of any kind.
 */
int al_server_loop_func (al_server_t *server)
{
   al_connection_t *c, *c_next;
   struct sockaddr_in client_addr;
   socklen_t client_addr_size;
   int fd, fd_max, res, bytes_read;

   /* before we wait, make sure our data is sane. */
   al_server_lock (server);
   server->state |= AL_SERVER_STATE_IN_LOOP;

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
   if (server->state & AL_SERVER_STATE_PIPE) {
      FD_SET (server->pipe_fd[0], &(server->fd_in));
      fd_max = AL_MAX (fd_max, server->pipe_fd[0]);
   }

   /* add all other connections. */
   for (c = server->connection_list; c != NULL; c = c->next) {
      /* unless we're closing, read from this. */
      if (!(c->flags & AL_CONNECTION_CLOSING))
         FD_SET (c->sock_fd, &(server->fd_in));

      /* if there's stuff to write, add to the write buffer. */
      al_connection_stage_output (c);
      if (c->flags & AL_CONNECTION_WRITING)
         FD_SET (c->sock_fd, &(server->fd_out));

      /* always add to the 'other' set. */
      FD_SET (c->sock_fd, &(server->fd_other));

      /* select() needs to know the highest file descriptor. */
      fd_max = AL_MAX (fd_max, c->sock_fd);
   }

   /* don't greedily lock the server while select() is waiting. */
   al_server_unlock (server);

   /* wait forever until we have some activity. */
   if ((res = select (fd_max + 1, &(server->fd_in), &(server->fd_out),
               &(server->fd_other), NULL)) == SOCKET_ERROR) {
      if (errno != EINTR)
         AL_ERROR ("select() error: %d\n", errno);
      server->state |= AL_SERVER_STATE_QUIT;
      server->state &= ~AL_SERVER_STATE_IN_LOOP;
      return 0;
   }

   /* lock our server. */
   al_server_lock (server);

   /* clear out data from our pipe. */
   if (server->state & AL_SERVER_STATE_PIPE)
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
               .connection   = c,
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

   /* check all of our connections for output and check
    * if they should be closed. */
   for (c = server->connection_list; c != NULL; c = c_next) {
      c_next = c->next;
      if (FD_ISSET (c->sock_fd, &(server->fd_out)))
         if (al_connection_fd_write (c) < 0) {
            al_connection_free (c);
            continue;
         }
      if (c->output_len == 0 && c->flags & AL_CONNECTION_CLOSING) {
         al_connection_free (c);
         continue;
      }
   }

   /* unlock server and return success. */
   server->state &= ~AL_SERVER_STATE_IN_LOOP;
   al_server_unlock (server);
   return 1;
}

/* al_server_pthread_func():
 * -------------------------
 * Function hook passed to our POSIX thread.  This function is the main server
 * loop, which will continuously manage connections via al_server_loop_func()
 * until given the shutdown notice via toggling AL_SERVER_QUIT in the server
 * state.  If the al_server_open() was called from al_server_start(), all
 * connections including the listening socket are closed.
 */
void *al_server_pthread_func (void *arg)
{
   al_server_t *server;
   server = arg;

   /* run until the server is told to quit. */
   while (!al_server_is_quitting (server))
      al_server_loop_func (server);

   /* perform clean up.  mark that we're no longer running. */
   al_server_lock (server);
   server->state &= ~AL_SERVER_STATE_RUNNING;

   /* if our connection was opened from al_server_start(), make sure we close
    * it here, too. */
   if (server->flags & AL_SERVER_CLOSE_AFTER_STOP) {
      al_server_close (server);
      server->flags &= ~AL_SERVER_CLOSE_AFTER_STOP;
   }
   al_server_unlock (server);

   /* we're done. */
   return NULL;
}

/* al_server_start():
 * -----------------
 * Start the server loop in a background thread called the 'server thread'.
 * For convenience, if the listening socket has not yet been opened, open it
 * here and give the responsibility to close it to the server thread.
 *
 * Returns 1 on success, 0 if the server was already running or the listening
 * socket couldn't be opened.
 */
int al_server_start (al_server_t *server)
{
   int res;

   /* don't do anything if the server is already running. */
   if (al_server_is_running (server))
      return 0;

   /* don't do anything if we couldn't open the server. */
   if (!al_server_is_open (server)) {
      if (!al_server_open (server))
         return 0;
      server->flags |= AL_SERVER_CLOSE_AFTER_STOP;
   }

   /* attempt to start a pthread. */
   server->state |= AL_SERVER_STATE_RUNNING;
   if ((res = pthread_create (&(server->pthread), NULL, al_server_pthread_func,
                              (void *) server)) != 0) {
      server->state &= ~(AL_SERVER_STATE_RUNNING | AL_SERVER_CLOSE_AFTER_STOP);
      AL_ERROR ("Unable to start server (Error: %d)\n", res);
      return 0;
   }

   /* return success. */
   return 1;
}

/* al_server_wait():
 * ------------------
 * Wait patiently for the server loop's thread to end from a shutdown signal.
 *
 * Return value: Returns 1 if the server shut down normally,
 *               returns 0 if the server wasn't running or if we're currently
 *                  in the server thread (this is an error).
 */
int al_server_wait (al_server_t *server)
{
   if (al_server_in_thread (server)) {
      AL_ERROR ("al_server_wait() called within server thread!\n");
      return 0;
   }
   if (!al_server_is_running (server))
      return 0;
   pthread_join (server->pthread, NULL);
   return 1;
}

/* al_server_interrupt():
 * ----------------------
 * Writes to the pipe created in al_server_open() in order to break out of
 * the select() call in al_server_loop_func().
 *
 * Return value: Returns 1 on success, 0 on any failure.
 */
int al_server_interrupt (al_server_t *server)
{
   /* must be running. */
   if (!al_server_is_running (server))
      return 0;

   /* must have a pipe we can send something to. */
   if (!(server->state & AL_SERVER_STATE_PIPE))
      return 0;

   /* write something! */
   if (write (server->pipe_fd[1], "\x01", 1) != 1) {
      AL_PRINTF ("server_interrupt() failed.\n");
      return 0;
   }

   return 1;
}

/* al_server_stop():
 * -----------------
 * Sends a shutdown signal to the server loop thread by toggling the
 * AL_SERVER_STATE_QUIT flag and sending an interrupt to the select() function
 * in al_server_loop_func().
 *
 * Return value: Returns 1 on success, 0 if the server is not running or
 *               already shutting down.
 */
int al_server_stop (al_server_t *server)
{
   if (!al_server_is_running (server))
      return 0;
   if (al_server_is_quitting (server))
      return 0;
   server->state |= AL_SERVER_STATE_QUIT;
   al_server_interrupt (server);
   return 1;
}

/* al_server_free():
 * -----------------
 * Stops the server loop, closes all connections, cleans up, deallocates all
 * memory used by the server, then destroys the server itself.
 */
int al_server_free (al_server_t *server)
{
   /* don't allow the server to be freed from within the server thread. */
   if (al_server_in_thread (server)) {
      AL_ERROR ("al_server_free() called from within server thread!\n");
      return 0;
   }

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

   /* get rid of all our modules. */
   while (server->module_list)
      al_module_free (server->module_list);

   /* free the server itself and return success. */
   free (server);
   return 1;
}

/* al_server_func_set():
 * ---------------------
 * Assigns user-defined function hooks to key tasks that need to be performed.
 * Most tasks involve handling incoming or outgoing network traffic from
 * client connections.
 *
 * server:   The server instance.
 * task:     The index of the function being modified (see list below).
 * func:     A function pointer for the task (see below for declaration).
 *
 * Return value: Returns 1 on success,
 *               returns 0 for invalid servers or tasks.
 *
 * Function hook type definition (al_server_func):
 * -----------------------------------------------
 * int foo (al_server_t *server, al_connection_t *connection, int func,
 *          void *arg)
 *
 * server:     The server instance.
 * connection: Connection relevant to the task required.
 * task:       Type of task calling the function hook.
 * arg:        Task-specific data.  Cast to relevant type before using.
 *
 * For convenience, a macro is provided to declare al_server_func's:
 * -----------------------------------------------------------------
 * AL_SERVER_FUNC (foo)  <-- parameters are (server, connection, func, arg)
 * {
 *    ...
 *    return some_int;
 * }
 *
 * Valid values for 'task', their argument (*arg), and return values:
 * ------------------------------------------------------------------
 * AL_SERVER_FUNC_JOIN:
 *    A new client has successfully connected.
 *    arg:          (unused)
 *    Return value: 1 if connection is valid, 0 if it should be closed.
 *
 * AL_SERVER_FUNC_LEAVE:
 *    A client has just disconnected.
 *    arg:          (unused)
 *    Return value: (unused)
 *
 * AL_SERVER_FUNC_READ:
 *    New input from a client is available for processing.
 *    arg:          al_func_read_t *
 *                  (see 'connections.h' for specification and 'read.c' for a
 *                   list of helper functions.)
 *    Return value: (unused)
 *
 * AL_SERVER_FUNC_PRE_WRITE:
 *    Output has been staged and is ready to be sent out.
 *    arg:          al_func_pre_write_t *
 *                  (see 'connections.h' for specification)
 *    Return value: (unused)
 */
int al_server_func_set (al_server_t *server, int task, al_server_func *func)
{
   /* error-checking. */
   if (server == NULL)
      return 0;
   if (task < 0 || task >= AL_SERVER_FUNC_MAX)
      return 0;

   /* safely set function and return success. */
   al_server_lock (server);
   server->func[task] = func;
   al_server_unlock (server);
   return 1;
}

/* al_server_write():
 * ------------------
 * Writes data to all connections on a server.
 *
 * server: The server instance.
 * buf:    Data to send out as unsigned bytes.
 * size:   Number of bytes to send out.
 *
 * Return value: The number of connections written to.
 */
int al_server_write (al_server_t *server, const unsigned char *buf,
   size_t size)
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

/* al_server_write_string():
 * -------------------------
 * Writes null-terminated strings to all connections on the server.  The
 * trailing '\0' character is not written.
 * 
 * server: The server instance.
 * string: Null-terminated string to write.
 *
 * Return value: The number of connections written to.
 */
int al_server_write_string (al_server_t *server, const char *string)
{
   return al_server_write (server, (unsigned char *) string, strlen (string));
}

/* al_server_module_new():
 * -----------------------
 * Simple server wrapper for al_module_new().  Assigns generic data to the
 * server and associates it with a name entry.
 *
 * server:    The server instance.
 * name:      Name of the new module.
 * data:      Data assigned to the module.  The module will automatically free
 *            this data when the module itself is freed.
 * free_func: Optional function called before freeing data in al_module_free().
 *            (see 'modules.c' for 'al_module_func' specification.)
 *
 * Returns: Pointer to new instance of 'al_module_t' or NULL on error.
 */
al_module_t *al_server_module_new (al_server_t *server, const char *name,
   void *data, size_t data_size, al_module_func *free_func)
{
   return al_module_new (server, &(server->module_list), name, data, data_size,
                         free_func);
}

/* al_server_module_get():
 * -----------------------
 * Simple server wrapper for al_module_get().  Looks up a module by name and
 * returns it if available.  Returns NULL if the module cannot be found.
 */
al_module_t *al_server_module_get (const al_server_t *server,
   const char *name)
   { return al_module_get (&(server->module_list), name); }

/* al_server_in_thread():
 * ----------------------
 * Check if pthread_self() matches the server thread.
 *
 * server: The server whose thread we're checking.
 *
 * Returns: 0 if the server is not running or pthread_self() doesn't match.
 *          1 if the server is running and pthread_self() matches.
 */
int al_server_in_thread (const al_server_t *server)
{
   if (!al_server_is_running (server))
      return 0;
   if (pthread_equal (pthread_self(), server->pthread))
      return 1;
   return 0;
}
