/* utils.c
 * -------
 * utility functions. */

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
   #include "config.h"
#endif

#include "alpaca/utils.h"

#ifndef HAVE_STRDUP
char *strdup (const char *str)
{
   if (str == NULL)
      return NULL;
   int len   = strlen (str) + 1;
   char *buf = malloc(len);
   memcpy (buf, str, len);
   return buf;
}
#endif
