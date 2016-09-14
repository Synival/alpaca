/* utils.h
 * -------
 * utility functions. */

#ifndef __ALPACA_C_UTILS_H
#define __ALPACA_C_UTILS_H

#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

#ifndef HAVE_STRDUP
#if (defined __APPLE__) || (defined _SVID_SOURCE) || (defined _BSD_SOURCE) || \
    (_XOPEN_SOURCE >= 500) || ((defined _XOPEN_SOURCE) && \
    (defined _XOPEN_SOURCE_EXTENDED)) || _POSIX_C_SOURCE >= 200809L
   #define HAVE_STRDUP
#endif
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
