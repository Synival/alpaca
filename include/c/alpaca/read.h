/* read.h
 * ------
 * various methods for reading from input buffers. */

#ifndef __ALPACA_C_READ_H
#define __ALPACA_C_READ_H

#include "defs.h"

/* data sent via AL_SERVER_READ_FUNC. */
struct _al_func_read_t {
   al_connection_t *connection;
   unsigned char *data, *new_data;
   size_t data_len, new_data_len, bytes_used;
};

/* functions. */
int al_read_used (al_func_read_t *read, size_t len);
size_t al_read_line (char *buf, size_t size, al_func_read_t *read);

#endif
