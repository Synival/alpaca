/* connections.c
 * -------------
 * connection management for servers. */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "alpaca/modules.h"
#include "alpaca/server.h"

#include "alpaca/connections.h"

al_connection_t *al_connection_new (al_server_t *server, int fd,
   const struct sockaddr_in *addr, socklen_t addr_size)
{
   al_connection_t *new;

   /* allocate and assign data. */
   new = calloc (1, sizeof (al_connection_t));
   new->sock_fd = fd;
   if (addr) {
      new->addr = *addr;
      new->addr_size = addr_size;
   }

   /* record IP address. */
   char address[INET_ADDRSTRLEN];
   inet_ntop (AF_INET, &(new->addr.sin_addr), address, INET_ADDRSTRLEN);
   new->ip_address = strdup (address);

   /* link to our server. */
   al_server_lock (server);
   AL_LL_LINK_FRONT (new, server, prev, next, server, connection_list);
   if (server->func[AL_SERVER_FUNC_JOIN])
      if (!server->func[AL_SERVER_FUNC_JOIN] (server, new,
           AL_SERVER_FUNC_JOIN, NULL)) {
         al_connection_free (new);
         return NULL;
      }
   al_server_unlock (server);

   /* return our new connection. */
   return new;
}

int al_connection_free (al_connection_t *c)
{
   al_server_t *server;

   /* boo race conditions! */
   server = c->server;
   al_server_lock (server);

   /* function for leaving? */
   if (server->func[AL_SERVER_FUNC_LEAVE])
      server->func[AL_SERVER_FUNC_LEAVE] (server, c, AL_SERVER_FUNC_LEAVE, 0);

   /* free all modules. */
   while (c->module_list)
      al_module_free (c->module_list);

   /* attempt to send remaining output. */
   al_connection_stage_output (c);
   al_connection_fd_write (c);

   /* close our socket. */
   socket_close (c->sock_fd);

   /* free all other allocated memory. */
   if (c->input)
      free (c->input);
   if (c->output)
      free (c->output);
   if (c->ip_address)
      free (c->ip_address);

   /* unlink. */
   AL_LL_UNLINK (c, prev, next, c->server, connection_list);

   /* free remaining data and return success. */
   free (c);
   al_server_unlock (server);
   return 1;
}

int al_connection_close (al_connection_t *c)
{
   /* fail if already being closed. */
   if (c->flags & AL_CONNECTION_CLOSING)
      return 0;
   c->flags |= AL_CONNECTION_CLOSING;
   return 1;
}

int al_connection_append_buffer (al_connection_t *c, unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, const unsigned char *input,
   size_t isize)
{
   size_t new_size;

   /* don't do anything if there's no buffer whatsoever. */
   if (input == NULL || isize == 0)
      return 0;

   /* don't allow reading while we're doing this. */
   al_server_lock (c->server);

   /* how large should our buffer be?  use a sensible size for starting. */
   if (*buf == NULL)
      new_size = 256;
   else
      new_size = *size;
   while (new_size < *len + isize + 1)
      new_size *= 2;

   /* make sure our buffer is the proper size. */
   if (*buf == NULL) {
      *buf = malloc (sizeof (unsigned char) * new_size);
      *size = new_size;
   }
   else if (new_size != *size) {
      *buf = realloc (*buf, new_size);
      *size = new_size;
   }

   /* copy data into our buffer.  make it NULL-terminated, just in case. */
   memcpy (*buf + *len, input, sizeof (unsigned char) * isize);
   *len += isize;
   *((*buf) + *len) = 0;

   /* we did it, you guise! */
   al_server_unlock (c->server);
   return 1;
}

int al_connection_fetch_buffer (al_connection_t *c, const unsigned char **buf,
   size_t *size, size_t *len, size_t *pos, unsigned char *output,
   size_t osize)
{
   size_t input_len;

   /* if we don't have a buffer, don't do anything. */
   if (output == NULL || osize == 0)
      return 0;

   /* don't allow writing while we're doing this. */
   al_server_lock (c->server);

   /* ...and don't bother if there's nothing to read. */
   input_len = *len - *pos;
   if (*buf == NULL || input_len <= 0) {
      al_server_unlock (c->server);
      return 0;
   }
   /* are we only reading a portion? */
   else if (osize < input_len) {
      memcpy (output, *buf + *pos, sizeof (unsigned char) * osize);
      *pos += osize;
      al_server_unlock (c->server);
      return osize;
   }
   /* looks like we're reading everything! */
   else {
      memcpy (output, *buf + *pos, sizeof (unsigned char) * input_len);

      /* reset our buffer and return the number of bytes we read. */
      *len = 0;
      *pos = 0;
      al_server_unlock (c->server);
      return input_len;
   }
}

int al_connection_read (al_connection_t *c, unsigned char *buf, size_t size)
{
   /* read from our auto-sizing input buffer. */
   return al_connection_fetch_buffer (c, (const unsigned char **) &(c->input),
      &(c->input_size), &(c->input_len), &(c->input_pos), buf, size);
}

int al_connection_fd_read (al_connection_t *c)
{
   static unsigned char buf[4096];
   int res;

   /* attempt to read.  if it didn't work, the connection has been closed.
    * return -1 to indicate an error. */
   if ((res = read (c->sock_fd, buf, 4096)) <= 0) {
      unsigned char *ib;
      ib = malloc (c->input_len + 1);
      memcpy (ib, c->input, c->input_len);
      ib[c->input_len] = '\0';
      free (ib);
      return -1;
   }

   /* add to our input buffer. */
   al_connection_append_buffer (c, &(c->input), &(c->input_size),
      &(c->input_len), &(c->input_pos), buf, res);

   /* return the number of bytes read. */
   return res;
}

int al_connection_fd_write (al_connection_t *c)
{
   /* output at most 'c->output_max' bytes.  bail if there's no work for us. */
   size_t bytes = c->output_len - c->output_pos,
          max   = c->output_max < bytes ? c->output_max : bytes;
   if (max <= 0 || !(c->flags & AL_CONNECTION_WRITING))
      return 0;

   /* attempt to write to the socket. */
   int res;
   if ((res = write (c->sock_fd, c->output + c->output_pos, bytes)) <= 0) {
      AL_ERROR ("Couldn't write %ld bytes to client [%d].\n", bytes,
                c->sock_fd);
      return -1;
   }

   /* if we wrote everything, clear out our buffer. */
   if (res >= bytes) {
      c->output_len = 0;
      c->output_pos = 0;
      c->flags &= ~AL_CONNECTION_WROTE;
   }
   /* otherwise, just move forward a little bit. */
   else
      c->output_pos += bytes;

   /* allow AL_SERVER_FUNC_PRE_WRITE to run again once output_max
    * reaches zero.  this way, if we've queued a massive amount of data for
    * output and, while sending it out, the output buffer grows larger,
    * the pre-write function will still be executed in its original place. */
   c->output_max -= res;
   if (c->output_max <= 0)
      c->flags &= ~AL_CONNECTION_WRITING;

   return bytes;
}

int al_connection_write (al_connection_t *c, const unsigned char *buf,
   size_t size)
{
   /* don't write blank data or to connections being closed. */
   if (size == 0 || c->flags & AL_CONNECTION_CLOSING)
      return 0;
   int res = al_connection_append_buffer (c, &(c->output), &(c->output_size),
      &(c->output_len), &(c->input_pos), buf, size);
   al_connection_wrote (c);
   return res;
}

int al_connection_write_string (al_connection_t *c, const char *string)
{
   /* send a string as unsigned bytes. */
   return al_connection_write (c, (unsigned char *) string,
                               strlen (string));
}

int al_connection_wrote (al_connection_t *c)
{
   /* mark that this connection is awaiting al_connection_stage_output(). */
   c->flags |= AL_CONNECTION_WROTE;
   al_server_interrupt (c->server);
   return 1;
}

int al_connection_stage_output (al_connection_t *c)
{
   /* don't do anything if it was never indicated that there's output to
    * write... */
   if (!(c->flags & AL_CONNECTION_WROTE))
      return -1;
   /* ...or if this function has already been called before writing. */
   else if (c->flags & AL_CONNECTION_WRITING)
      return -1;

   /* call the 'pre_write' function if available. */
   if (c->server->func[AL_SERVER_FUNC_PRE_WRITE]) {
      al_func_pre_write_t data = {
         .data     = c->output,
         .data_len = c->output_len
      };
      c->server->func[AL_SERVER_FUNC_PRE_WRITE] (c->server, c,
         AL_SERVER_FUNC_PRE_WRITE, &data);
   }

   /* make that there's data to write out and record/return the byte count. */
   c->flags |= AL_CONNECTION_WRITING;
   c->output_max = c->output_len - c->output_pos;
   return c->output_max;
}

al_module_t *al_connection_module_new (al_connection_t *connection,
   const char *name, void *data, size_t data_size, al_module_func *free_func)
{
   return al_module_new (connection, &(connection->module_list), name, data,
      data_size, free_func);
}
al_module_t *al_connection_module_get (const al_connection_t *connection,
   const char *name)
   { return al_module_get (&(connection->module_list), name); }
