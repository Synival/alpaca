#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alpaca/alpaca.h>

AL_HTTP_FUNC (example_http_error)
{
   al_http_header_response_set (request, "Content-Type", "text/html");
   al_http_write_string (request,
      "<!doctype html>\n"
      "<html>\n"
      "<body>\n"
      "<h1>This request doesn't work!</h1>\n"
      "<p>Whatever you did, you did it wrong!\n"
      "</body>\n"
      "</html>\n");
   return 0;
}

AL_HTTP_FUNC (example_http_get)
{
   /* simulate an error if our URI is 'error'. */
   if (al_uri_path_is (request->uri->path, "error", NULL)) {
      al_http_set_status_code (request, 200);
      return example_http_error (request, func, data);
   }
   /* return a no-content page, whose response code should be 204. */
   else if (al_uri_path_is (request->uri->path, "no_content", NULL)) {
      al_http_set_status_code (request, 204);
      return 0;
   }
   /* return a blank page (different from 'no_content'). */
   else if (al_uri_path_is (request->uri->path, "blank", NULL))
      return 0;

   char html[8192];

   /* build a simple HTML page. */
   al_http_header_response_set (request, "Content-Type", "text/html");
   snprintf (html, sizeof (html),
      "<!doctype html>\n"
      "<html>\n"
      "<body>\n"
      "<h1>%s request:</h1>\n"
      "<table>\n"
      "  <tr><td><b>URI</b>:</td><td>%s</td></tr>\n"
      "  <tr><td><b>Version</b>:</td><td>%s</td></tr>\n"
      "</table>\n",
      request->verb, request->uri_str, request->version_str);
   al_http_write_string (request, html);

   /* barf all the header info back to the client. */
   al_http_header_t *h;
   al_http_write_string (request,
      "<h1>Header:</h1>\n"
      "<table>\n");
   for (h = request->header_request; h != NULL; h = h->next) {
      snprintf (html, sizeof (html),
         "  <tr><td><b>%s</b>:</td><td>%s</td></tr>\n", h->name, h->value);
      al_http_write_string (request, html);
   }
   al_http_write_string (request, "</table>\n");

   /* write detailed URI path information... */
   al_http_write_string (request,
      "<h1>URI Path:</h1>\n"
      "<ul>\n");
   al_uri_path_t *p;
   for (p = request->uri->path; p != NULL; p = p->next) {
      snprintf (html, sizeof (html), "   <li>%s</li>\n",
         (p->name[0] != '\0') ? p->name : "<i>(default)</i>");
      al_http_write_string (request, html);
   }
   al_http_write_string (request, "</ul>\n");

   /* ...and the URI query. */
   al_http_write_string (request,
      "<h1>URI Query:</h1>\n"
      "<ul>\n");
   al_uri_parameter_t *q;
   for (q = request->uri->parameters; q != NULL; q = q->next) {
      snprintf (html, sizeof (html), "   <li>%s = %s</li>\n",
         q->name, q->value);
      al_http_write_string (request, html);
   }
   al_http_write_string (request, "</ul>\n");

   /* end our HTML. */
   snprintf (html, sizeof (html),
      "</body>\n"
      "</html>\n");
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
