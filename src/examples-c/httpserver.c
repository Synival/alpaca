#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alpaca/alpaca.h>

AL_HTTP_FUNC (example_http_get)
{
   char buf[4096], *ch;
   if ((ch = strchr (data, ' ')) != NULL)
      *ch = '\0';
   snprintf (buf, sizeof (buf),
      "<!doctype html>\n"
      "<html>\n"
      "<body>\n"
      "<h1>Wow!</h1>\n"
      "Your path: %s\n"
      "</body>\n"
      "</html>\n", data);
   al_connection_write_string (connection, buf);
   return 1;
}

int main (int argc, char **argv)
{
   /* requires at least 1 parameter (port). */
   if (argc < 2) {
      fprintf (stderr, "Usage: httpserver <port>\n");
      return 1;
   }

   /* get a port.  default to 4096. */
   int port = atoi (argv[1]);
   printf ("Will run on port %d.\n", port);

   /* instantiate our server with basic flags. */
   al_server_t *server = al_server_new (port, 0);

   /* use an HTTP module and assign some basic functions to it. */
   al_http_t *http = al_http_init (server);
   al_http_set_func (http, "GET", example_http_get);

   /* start our server. */
   if (!al_server_start (server)) {
      fprintf (stderr, "Server failed to start.\n");
      return 2;
   }
   printf (
"=========================================================================\n"
"  Server running and waiting for API calls.\n"
"=========================================================================\n");

   /* shut down the server as soon as its stopped via 'shutdown'. */
   al_server_wait (server);
   printf ("Shutdown signal recieved.  Freeing server.\n");
   al_server_free (server);
   printf ("Server freed successfully.  Exiting.\n");

   /* return success. */
   return 0;
}
