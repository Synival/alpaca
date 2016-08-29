/* read.h
 * ------
 * various methods for reading from input buffers. */

#ifndef __ALPACA_C_READ_H
#define __ALPACA_C_READ_H

#include "defs.h"

/* functions. */
int al_read_used (al_func_read_t *read, size_t len);
size_t al_read_line (char *buf, size_t size, al_func_read_t *read);

#endif
