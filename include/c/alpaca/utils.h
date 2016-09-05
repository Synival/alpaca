/* utils.h
 * -------
 * utility functions. */

#ifndef __ALPACA_C_UTILS_H
#define __ALPACA_C_UTILS_H

#ifdef HAVE_CONFIG_H
   #include "config.h"
#elif defined __APPLE__
   #define HAVE_STRDUP
#endif

#define AL_PRINTF  printf
#define AL_FPRINTF fprintf
#define AL_ERROR(...) \
   AL_FPRINTF (stderr, __VA_ARGS__)

#define AL_MAX(x, y) \
   (((x) > (y)) ? (x) : (y))
#define AL_MIN(x, y) \
   (((x) < (y)) ? (x) : (y))

#ifndef HAVE_STRDUP
char *strdup (const char *str);
#endif

#endif
