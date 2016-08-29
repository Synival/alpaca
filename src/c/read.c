/* read.c
 * ------
 * various methods for reading from input buffers. */

#include <string.h>

#include "alpaca/connections.h"

#include "alpaca/read.h"

int al_read_used (al_func_read_t *read, size_t len)
{
   /* pedantic error-checking. */
   if (len > read->data_len) {
      AL_ERROR ("al_read_used(): attempted to use %d more bytes than "
                "available!\n", (int) (len - read->data_len));
      len = read->data_len;
   }

   /* move our buffers forward. */
   read->data     += len;
   read->data_len -= len;
   if (read->new_data < read->data) {
      read->new_data     = read->data;
      read->new_data_len = read->data_len;
   }

   /* record the number of bytes used thus far and return how much we just
    * increased it by. */
   read->bytes_used += len;
   return len;
}

size_t al_read_line (char *buf, size_t size, al_func_read_t *read)
{
   /* eat up initial '\0' characters. */
   size_t pos = 0;
   while (pos < read->data_len && ((char *) read->data)[pos] == '\0')
      pos++;
   al_read_used (read, pos);

   /* make sure there's a line to read. */
   if (read->data_len == 0)
      return 0;

   /* cast bytes to signed string. */
   char *string = (char *) read->data;

   /* look for a '\r', '\n', or '\0'.  if none are present, do nothing. */
   for (pos = (size_t) (read->new_data - read->data);
        pos < read->data_len; pos++)
      if (string[pos] == '\r' || string[pos] == '\n' || string[pos] == '\0')
         break;
   if (pos == read->data_len)
      return 0;

   /* we found one, and it's at 'pos'.  record this as our line length. */
   size_t len = pos;

   /* replace ('\r\n') with '\0' and ('\r' or '\n') with '\0'.
    * first, check for '\r\n'. */
   /* FIXME: if the '\n' hasn't been read yet, this will cause the next
    * call of al_read_line() to find an empty line in quick succession. */
   if (len + 1 < read->data_len && string[len + 0] == '\r' &&
                                   string[len + 1] == '\n') {
      string[len++] = '\0';
      string[len++] = '\0';
   }
   /* replacing only one character. */
   else
      string[len++] = '\0';

   /* write to our output buffer.  make sure we don't exceed its length.
    * always cap it off with a '\0'. */
   if (len <= size)
      memcpy (buf, read->data, len);
   else {
      memcpy (buf, read->data, size - 1);
      buf[size - 1] = '\0';
   }

   /* record the bytes that we used. */
   len = al_read_used (read, len);

   /* return the length of the line (including crlf). */
   return len;
}
