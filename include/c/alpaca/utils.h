/* utils.h
 * -------
 * utility functions. */

#ifndef __ALPACA_C_UTILS_H
#define __ALPACA_C_UTILS_H

#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

/* check manually for strdup() - configure script makes mistakes. */
#if (defined __APPLE__) || \
    (defined _SVID_SOURCE) || \
    (defined _BSD_SOURCE) || \
    (_XOPEN_SOURCE >= 500) || \
    ((defined _XOPEN_SOURCE) && (defined _XOPEN_SOURCE_EXTENDED)) || \
    (_POSIX_C_SOURCE >= 200809L)
   #ifndef HAVE_STRDUP
      #define HAVE_STRDUP 1
   #endif
#else
   #undef HAVE_STRDUP
#endif

/* add strdup() if we have to. */
#ifndef HAVE_STRDUP
   char *strdup (const char *str);
#else
   #include <string.h>
#endif

/* add timeradd() if we need to. */
#ifndef HAVE_TIMERADD
   #define timeradd(a, b, result)                         \
      do {                                                \
         (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
         (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
         if ((result)->tv_usec >= 1000000) {              \
            ++(result)->tv_sec;                           \
            (result)->tv_usec -= 1000000;                 \
         }                                                \
      } while (0)
#endif

/* add timersub() if we need to. */
#ifndef HAVE_TIMERSUB
   #define timersub(a, b, result)                         \
      do {                                                \
         (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
         (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
         if ((result)->tv_usec < 0) {                     \
            --(result)->tv_sec;                           \
            (result)->tv_usec += 1000000;                 \
         }                                                \
      } while (0)
#endif

/* add timercmp() if we need to. */
#ifndef HAVE_TIMERCMP
   #define timercmp(a, b, CMP)           \
      (((a)->tv_sec  ==  (b)->tv_sec)  ? \
       ((a)->tv_usec CMP (b)->tv_usec) : \
       ((a)->tv_sec  CMP (b)->tv_sec))
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
