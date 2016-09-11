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

#ifndef HAVE_STRDUP
char *strdup (const char *str);
#endif

/* our own functions. */
int al_util_replace_string (char **dst, const char *src);

/* handy macros. */
#define AL_PRINTF  printf
#define AL_FPRINTF fprintf
#define AL_ERROR(...) \
   AL_FPRINTF (stderr, __VA_ARGS__)

#define AL_MAX(x, y) \
   (((x) > (y)) ? (x) : (y))
#define AL_MIN(x, y) \
   (((x) < (y)) ? (x) : (y))

#endif
