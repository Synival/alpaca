#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alpaca/alpaca.h>

AL_HTTP_FUNC (example_http_get)
{
   char html[8192];
   size_t len;

   /* build a simple HTML page. */
   len = snprintf (html, sizeof (html),
      "<!doctype html>\r\n"
      "<html>\r\n"
      "<body>\r\n"
      "<h1>%s request:</h1>\r\n"
      "<table>\r\n"
      "  <tr><td><b>URI</b>:</td><td>%s</td></tr>\r\n"
      "  <tr><td><b>Version</b>:</td><td>%s</td></tr>\r\n"
      "</table>\r\n"
      "<h1>Header:</h1>\r\n"
      "<table>\r\n", state->verb, state->uri, state->version_str);

   /* barf all the header info back to the client. */
   al_http_header_t *h;
   for (h = state->header_list; h != NULL; h = h->next)
      len += snprintf (html+ len, sizeof (html) - len,
         "  <tr><td><b>%s</b>:</td><td>%s</td></tr>\r\n", h->name, h->value);

   /* end our HTML. */
   snprintf (html + len, sizeof (html) - len,
      "</table>\r\n"
      "</body>\r\n"
      "</html>\r\n");

   /* build our header AFTER the file so we have 'Content-Length'. */
   char header[8192];
   snprintf (header, sizeof (header),
      "HTTP/1.1 200 OK\r\n"
      "Cache-Control: no-cache\r\n"
      "Connection: close\r\n"
      "Content-Length: %d\r\n"
      "\r\n", strlen (html));

   /* send out our header and web content. */
   al_connection_write_string (connection, header);
   al_connection_write_string (connection, html);

   /* return success. */
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
