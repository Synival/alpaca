#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alpaca/alpaca.h>

AL_HTTP_FUNC (example_http_error)
{
   al_http_write_string (request,
      "<!doctype html>\r\n"
      "<html>\r\n"
      "<body>\r\n"
      "<h1>This request doesn't work!</h1>\r\n"
      "<p>Whatever you did, you did it wrong!\r\n"
      "</body>\r\n"
      "</html>\r\n");
   return 0;
}

AL_HTTP_FUNC (example_http_get)
{
   /* simulate an error if our URI is 'error'. */
   if (strcmp (request->uri, "/error") == 0) {
      al_http_set_status_code (request, 200);
      return example_http_error (request, func, data);
   }

   char html[8192];

   /* build a simple HTML page. */
   snprintf (html, sizeof (html),
      "<!doctype html>\r\n"
      "<html>\r\n"
      "<body>\r\n"
      "<h1>%s request:</h1>\r\n"
      "<table>\r\n"
      "  <tr><td><b>URI</b>:</td><td>%s</td></tr>\r\n"
      "  <tr><td><b>Version</b>:</td><td>%s</td></tr>\r\n"
      "</table>\r\n"
      "<h1>Header:</h1>\r\n"
      "<table>\r\n", request->verb, request->uri, request->version_str);
   al_http_write_string (request, html);

   /* barf all the header info back to the client. */
   al_http_header_t *h;
   for (h = request->header_list; h != NULL; h = h->next) {
      snprintf (html, sizeof (html),
         "  <tr><td><b>%s</b>:</td><td>%s</td></tr>\r\n", h->name, h->value);
      al_http_write_string (request, html);
   }

   /* end our HTML. */
   snprintf (html, sizeof (html),
      "</table>\r\n"
      "</body>\r\n"
      "</html>\r\n");
   al_http_write_string (request, html);

   /* return success. */
   return 0;
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
   al_http_set_func (http, "GET",   example_http_get);
   al_http_set_func (http, "ERROR", example_http_error);

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
