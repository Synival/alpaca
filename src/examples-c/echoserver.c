#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alpaca/alpaca.h>

int example_broadcast (al_connection_t *c, char quote, char *message)
{
   char buf[256];
   int count;

   /* different message depending on our quotes. */
   if (quote != '\0')
      snprintf (buf, sizeof (buf), "[%d][%s] %c%s%c\r\n",
         c->sock_fd, c->ip_address, quote, message, quote);
   else
      snprintf (buf, sizeof (buf), "[%d][%s] %s\r\n",
         c->sock_fd, c->ip_address, message);

   /* broadcast to the server and all clients. */
   printf ("%s", buf);
   count = al_server_write_string (c->server, buf);

   /* return the number of clients who received this message. */
   return count;
}

AL_SERVER_FUNC (example_join)
{
   /* a friendly welcome message. */
   al_connection_write_string (connection,
"=========================================================================\n"
"Welcome to echoserver.  Everything you type will be echoed by the server.\n"
"=========================================================================\n"
      );

   /* let the server and all current connections know about this new
    * connection. */
   example_broadcast (connection, '\0', "(joined)");

   /* return 1 to indicate everything worked. */
   return 1;
}

AL_SERVER_FUNC (example_leave)
{
   /* inform the server that this connection has been closed. */
   example_broadcast (connection, '\0', "(left)");
   return 1;
}

AL_SERVER_FUNC (example_read)
{
   /* read lines until we can't anymore. */
   char buf[256];
   while (al_read_line (buf, sizeof (buf), data)) {
      /* print a message on the server and send it to everyone. */
      example_broadcast (connection, '\'', buf);

      /* if our client typed 'shutdown', start closing down. */
      if (strcmp (buf, "shutdown") == 0) {
         /* TODO: this is not yet being received by all clients. */
         al_server_write_string (connection->server, "Shutting down!\n");
         al_server_stop (connection->server);
         return 0;
      }
   }

   /* return value is irrelevant. */
   return 0;
}

int main (int argc, char **argv)
{
   /* requires at least 1 parameter (port). */
   if (argc < 2) {
      fprintf (stderr, "Usage: echoserver <port>\n");
      return EINVAL;
   }

   /* get a port.  default to 4096. */
   int port = atoi (argv[1]);
   printf ("Will run on port %d.\n", port);

   /* launch an alpaca server. */
   al_server_t *server = al_server_new (port, 0);
   al_server_func_set (server, AL_SERVER_FUNC_JOIN,  example_join);
   al_server_func_set (server, AL_SERVER_FUNC_LEAVE, example_leave);
   al_server_func_set (server, AL_SERVER_FUNC_READ,  example_read);
   if (!al_server_start (server)) {
      fprintf (stderr, "Server failed to start.\n");
      return EINVAL;
   }
   printf (
"=========================================================================\n"
"Server running and waiting for connections.\n"
"Type 'shutdown' from any client to stop the server.\n"
"=========================================================================\n");

   /* shut down the server as soon as its stopped via 'shutdown'. */
   al_server_wait (server);
   printf ("Shutdown signal recieved.  Freeing server.\n");
   al_server_free (server);
   printf ("Server freed successfully.  Exiting.\n");

   /* return success. */
   return 0;
}
