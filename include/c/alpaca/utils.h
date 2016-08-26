/* utils.h
 * -------
 * utility functions. */

#ifndef __ALPACA_C_UTILS_H
#define __ALPACA_C_UTILS_H

#define PRINTF  printf
#define FPRINTF fprintf
#define ERROR(...) \
   FPRINTF (stderr, __VA_ARGS__)

#define MAX(x, y) \
   (((x) > (y)) ? (x) : (y))
#define MIN(x, y) \
   (((x) < (y)) ? (x) : (y))

#endif
