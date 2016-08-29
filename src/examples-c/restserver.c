#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alpaca/alpaca.h>

int main (int argc, char **argv)
{
   /* requires at least 1 parameter (port). */
   if (argc < 2) {
      fprintf (stderr, "Usage: restserver <port>\n");
      return 1;
   }

   /* get a port.  default to 4096. */
   int port = atoi (argv[1]);
   printf ("Will run on port %d.\n", port);

   /* launch an alpaca server. */
   al_server_t *server = al_server_new (port, AL_SERVER_REST);
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
