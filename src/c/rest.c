/* rest.c
 * ------
 * RESTful API development tools. */

#include "alpaca/connections.h"
#include "alpaca/modules.h"
#include "alpaca/read.h"
#include "alpaca/server.h"

#include "alpaca/rest.h"

int al_rest_init (al_server_t *server)
{
   /* don't initialize if already initialized. */
   if (al_module_get (&(server->module_list), "rest")) {
      AL_ERROR ("al_rest_init(): REST module already initialized.\n");
      return 0;
   }

   /* create our module and set our own server function hooks. */
   al_server_module_new (server, "rest", NULL, 0, NULL);
   al_server_func_set (server, AL_SERVER_FUNC_READ, al_rest_func_read);

   /* return success. */
   return 1;
}

AL_SERVER_FUNC (al_rest_func_read)
{
   al_module_t *m = al_server_module_get (connection->server, "rest");
   int count = 0;

   /* read and print lines. */
   char buf[256];
   count = 0;
   while (al_read_line (buf, sizeof (buf), arg) > 0) {
      printf ("<%s> [%s]\n", m->name, buf);
      count++;
   }

   /* return non-error. */
   return 0;
}
