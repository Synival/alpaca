/* http.c
 * ------
 * HTTP API development tools. */

#include <stdlib.h>
#include <string.h>

#include "alpaca/connections.h"
#include "alpaca/modules.h"
#include "alpaca/read.h"
#include "alpaca/server.h"

#include "alpaca/http.h"

al_http_t *al_http_init (al_server_t *server)
{
   /* don't initialize if already initialized. */
   if (al_server_module_get (server, "http")) {
      AL_ERROR ("al_http_init(): HTTP module already initialized.\n");
      return NULL;
   }

   /* allocate some HTTP data whose ownership will be transfered to
    * the HTTP module. */
   al_http_t *http_data = calloc (1, sizeof (al_http_t));
   http_data->server = server;

   /* create our module and set our own server function hooks. */
   al_module_t *module = al_server_module_new (server, "http", http_data,
      sizeof (al_http_t), al_http_data_free);
   al_server_func_set (server, AL_SERVER_FUNC_READ,  al_http_func_read);
   al_server_func_set (server, AL_SERVER_FUNC_JOIN,  al_http_func_join);
   al_server_func_set (server, AL_SERVER_FUNC_LEAVE, al_http_func_leave);

   /* data we can set now that the module exists. */
   http_data->module = module;

   /* return our HTTP data. */
   return http_data;
}

AL_MODULE_FUNC (al_http_data_free)
{
   al_http_t *http = arg;
   while (http->func_list)
      al_http_free_func (http->func_list);
   return 0;
}

int al_http_state_cleanup (al_http_state_t *state)
{
   if (state->verb)        {free (state->verb);       state->verb       =NULL;}
   if (state->uri)         {free (state->uri);        state->uri        =NULL;}
   if (state->version_str) {free (state->version_str);state->version_str=NULL;}
   return 1;
}

AL_MODULE_FUNC (al_http_state_data_free)
{
   al_http_state_t *state = arg;
   al_http_state_cleanup (state);
   return 0;
}

AL_SERVER_FUNC (al_http_func_read)
{
   al_http_t       *http  = al_http_get (server);
   al_http_state_t *state = al_http_get_state (connection);
   int count = 0;

   /* read lines as long as the connection is alive. */
   char buf[256];
   while (al_read_line (buf, sizeof (buf), arg) > 0) {
      count++;
      printf ("   %s\n", buf);
      switch (state->state) {
         case AL_STATE_METHOD:
            al_http_state_method (connection, http, state, buf);
            break;
         case AL_STATE_HEADER:
            al_http_state_header (connection, http, state, buf);
            break;
      }
   }

   /* return non-error. */
   return 0;
}

int al_http_state_method (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state, char *line)
{
   /* make sure there's at least a verb and a URI. */
   char *verb = line, *uri;
   if ((uri = strchr (line, ' ')) == NULL)
      return 0;
   while (*uri == ' ') {
      *uri = '\0';
      uri++;
   }
   if (*uri == '\0')
      return 0;

   /* is there an HTTP version string? */
   char *version_str;
   if ((version_str = strchr (uri, ' ')) != NULL) {
      while (*version_str == ' ') {
         *version_str = '\0';
         version_str++;
      }
      if (*version_str == '\0')
         version_str = NULL;
   }

   /* get the version based on the version string. */
   int version = AL_HTTP_INVALID;
   if (version_str == NULL || strcmp (version_str, "HTTP/0.9") == 0)
      version = AL_HTTP_0_9;
   else if (strcmp (version_str, "HTTP/1.0") == 0)
      version = AL_HTTP_1_0;
   else if (strcmp (version_str, "HTTP/1.1") == 0)
      version = AL_HTTP_1_1;

   /* make sure a function exists for this verb. */
   al_http_func_def_t *fd;
   if ((fd = al_http_get_func (http, verb)) == NULL)
      return 0;

   /* remember strings and version info. */
   al_util_replace_string (&(state->verb),        verb);
   al_util_replace_string (&(state->uri),         uri);
   al_util_replace_string (&(state->version_str), version_str);
   state->version = version;

   /* behavior is different now depending on version. */
   switch (state->version) {
      case AL_HTTP_0_9:
         al_http_state_finish (connection, http, state);
         break;
      case AL_HTTP_1_0:
         state->state = AL_STATE_HEADER;
         break;
      case AL_HTTP_1_1:
         state->flags |= AL_STATE_PERSIST;
         state->state  = AL_STATE_HEADER;
         break;
      case AL_HTTP_INVALID:
         /* TODO: error. */
         return 0;
   }

   /* return success. */
   return 1;
}

int al_http_state_header (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state, char *line)
{
   /* if this is the last line, finish our request. */
   if (*line == '\0') {
      al_http_state_finish (connection, http, state);
      return 1;
   }

   /* TODO: build our header. */
   return 0;
}

AL_SERVER_FUNC (al_http_func_join)
{
   /* log everything. */
   AL_PRINTF ("JOIN:  #%d (%s)\n", connection->sock_fd,
      connection->ip_address);

   /* initialize a blank state for our HTTP request. */
   al_http_state_t *state = calloc (1, sizeof (al_http_state_t));
   state->connection = connection;
   al_http_state_reset (connection, al_http_get (server), state);

   /* assign the http data and return success. */
   al_connection_module_new (connection, "http", state,
      sizeof (al_http_state_t), al_http_state_data_free);
   return 1;
}

int al_http_state_reset (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state)
{
   al_http_state_cleanup (state);
   state->state   = AL_STATE_METHOD;
   state->version = AL_HTTP_INVALID;
   state->flags   = 0;
   return 1;
}

AL_SERVER_FUNC (al_http_func_leave)
{
   /* log everything. */
   AL_PRINTF ("LEAVE: #%d (%s)\n", connection->sock_fd,
      connection->ip_address);
   return 0;
}

al_http_t *al_http_get (al_server_t *server)
   { return al_server_module_get (server, "http")->data; }
al_http_state_t *al_http_get_state (al_connection_t *connection)
   { return al_connection_module_get (connection, "http")->data; }

al_http_func_def_t *al_http_set_func (al_http_t *http, char *verb,
   al_http_func *func)
{
   /* make sure this function doesn't already exist. */
   al_http_func_def_t *fd;
   if ((fd = al_http_get_func (http, verb)) != NULL)
      al_http_free_func (fd);

   /* create a new function definition. */
   al_http_func_def_t *new = calloc (1, sizeof (al_http_func_def_t));
   new->http = http;
   new->verb = strdup (verb);
   new->func = func;

   /* link it to our al_http_t. */
   AL_LL_LINK_FRONT (new, http, prev, next, http, func_list);

   /* return our new al_http_func_def_t. */
   return new;
}

al_http_func_def_t *al_http_get_func (al_http_t *http, char *verb)
{
   al_http_func_def_t *fd;
   if (http == NULL || verb == NULL)
      return NULL;
   for (fd = http->func_list; fd != NULL; fd = fd->next)
      if (strcmp (fd->verb, verb) == 0)
         return fd;
   return NULL;
}

int al_http_free_func (al_http_func_def_t *fd)
{
   /* free internal data. */
   if (fd->verb)
      free (fd->verb);

   /* unlink from the al_http_t. */
   AL_LL_UNLINK (fd, prev, next, fd->http, func_list);

   /* free structure itself and return success. */
   free (fd);
   return 1;
}

int al_http_state_finish (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state)
{
   /* TODO: check for necessary stuff in the header.
    * HTTP/1.1 requires a Host, for example. */

   /* is there a valid verb? */
   al_http_func_def_t *fd = al_http_get_func (http, state->verb);
   if ((fd = al_http_get_func (http, state->verb)) == NULL) {
      /* TODO: error. */
      return 0;
   }

   /* TODO: run whatever base function we should.  For HTTP/0.9, this is
    * fine, but for 1.0 and 1.1, it needs to send a return code and other
    * junk.  It should also have separate functions for bad requests. */

   /* run our function. */
   int r = fd->func (connection->server, connection, http, state, fd,
      state->uri);

   /* should this connection be closed or kept alive? */
   if (state->flags & AL_STATE_PERSIST)
      al_http_state_reset (connection, http, state);
   else
      al_connection_close (connection);

   /* return whatever our function returned. */
   return r;
}

int al_http_write_string (al_connection_t *connection, al_http_t *http,
   al_http_state_t *state, char *string)
{
   /* TODO: eventually, there might be gzip compression or other
    * considerations.  this function exists to make sure that can happen
    * if/when the feature exists. */
   return al_connection_write_string (connection, string);
}
