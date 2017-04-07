/* uri.c
 * -----
 * URI interpretation and break-down for requests. */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alpaca/uri.h"

al_uri_t *al_uri_new (const char *string)
{
   /* create a structure to contain our helpful URI data. */
   al_uri_t *new = calloc (1, sizeof (al_uri_t));

   /* set strings. */
   new->str_full = strdup (string);

   /* is there a query? */
   char *q = strchr (string, '?');
   if (q != NULL) {
      size_t len;

      /* get the 'path' part (everything up until '?')... */
      len = (q - string);
      new->str_path = malloc (sizeof (char) * (len + 1));
      memcpy (new->str_path, string, len);
      new->str_path[len] = '\0';

      /* ...and the 'query' part (everything after '?'). */
      q++;
      len = strlen (q);
      new->str_query = malloc (sizeof (char) * (len + 1));
      memcpy (new->str_query, q, len);
      new->str_query[len] = '\0';
   }
   /* no query - just copy the full thing. */
   else {
      new->str_path  = strdup (string);
      new->str_query = NULL;
   }

   /* variables used for tokenization coming up. */
   char *str, *pos, *next;
   int illegal = 0;

   /* tokenize our path with '/' as a delimiter. add 'al_path_t' structs. */
   if (new->str_path) {
      char decoded[256];
      al_uri_path_t *p = NULL;

      /* if the path doesn't start with '/', it's a relative path. */
      str = strdup (new->str_path);
      pos = str;
      if (pos[0] == '/')
         pos++;
      else
         new->flags |= AL_URI_RELATIVE;

      /* build path. */
      for (; pos != NULL; pos = next) {
         /* forbid paths that start with a period. */
         if (*pos == '.') {
            illegal = 1;
            break;
         }

         /* cap off our token and position the next one. */
         if ((next = strchr (pos, '/')) != NULL)
            { *next = '\0'; next++; }

         /* decode the entire token. */
         if (!al_uri_decode (pos, decoded, sizeof (decoded))) {
            illegal = 1;
            break;
         }

         /* append node to our path. */
         p = al_uri_path_append (new, p, decoded);
      }
      free (str);
   }

   /* tokenize our query with either '&' or ';' as delimiters. */
   if (new->str_query && !illegal) {
      char *eq, *left, *right, decoded_l[256], decoded_r[256];
      al_uri_parameter_t *p = NULL;

      str = strdup (new->str_query);
      for (pos = str; pos != NULL; pos = next) {
         if ((next = strpbrk (pos, ";&")) != NULL)
            { *next = '\0'; next++; }
         if (pos[0] == '\0')
            continue;

         /* is there an equal sign in our parameter? if no, value is 'true'. */
         if ((eq = strchr (pos, '=')) == NULL) {
            left  = pos;
            right = "true";
         }
         else {
            *eq = '\0';
            left  = pos;
            right = eq + 1;
         }

         /* decode left and right sides of the parameter. */
         if (!al_uri_decode (left,  decoded_l, sizeof (decoded_l)) ||
             !al_uri_decode (right, decoded_r, sizeof (decoded_r))) {
            illegal = 1;
            break;
         }

         /* append the name+value pair to our parameter list. */
         p = al_uri_parameter_append (new, p, decoded_l, decoded_r);
      }
      free (str);
   }

   /* if, after all this work, it was an invalid URI, undo all of our
    * hard work and return NULL. */
   if (illegal) {
      al_uri_free (new);
      return NULL;
   }

   /* return the new URI. */
   return new;
}

int al_uri_free (al_uri_t *uri)
{
   /* free path nodes and parameters. */
   while (uri->path)
      al_uri_path_free (uri->path);
   while (uri->parameters)
      al_uri_parameter_free (uri->parameters);

   /* free allocated strings. */
   if (uri->str_path)
      free (uri->str_path);
   if (uri->str_query)
      free (uri->str_query);

   /* free the structure itself and return success. */
   free (uri);
   return 1;
}

char *al_uri_decode (const char *input, char *output, size_t output_size)
{
   /* in ==> out.  precalculate some convenience string lengths. */
   size_t in,  input_len = strlen (input),
          out, output_len = output_size - 1;

   /* build our new string, one character at a time. */
   int h1, h2;
   for (in = 0, out = 0; in < input_len && out < output_len; in++, out++) {
      /* is this a hex code? */
      if (input[in] == '%') {
         /* if it's terminated prematurely, abort. */
         if (!(in + 2 < input_len))
            return NULL;

         /* get the hex code.  report an error if it's invalid. */
         if (!isxdigit ((h1 = toupper (input[in + 1])))) return NULL;
         if (!isxdigit ((h2 = toupper (input[in + 2])))) return NULL;
         if (h1 >= 'A') h1 = (h1 - 'A' + 10); else h1 = (h1 - '0');
         if (h2 >= 'A') h2 = (h2 - 'A' + 10); else h2 = (h2 - '0');
         output[out] = (h1 << 4) | h2;

         /* skip the two extra characters. */
         in += 2;
      }
      else
         output[out] = input[in];
   }

   /* terminate our output string. */
   output[out] = '\0';

   /* return our output for convenience. */
   return output;
}

al_uri_path_t *al_uri_path_append (al_uri_t *uri, al_uri_path_t *prev,
   const char *name)
{
   /* initialize a path node with a name. */
   al_uri_path_t *new = calloc (1, sizeof (al_uri_path_t));
   new->name = strdup (name);

   /* link it. */
   new->uri = uri;
   if (prev) {
      prev->next = new;
      new->prev = prev;
   }
   else
      uri->path = new;

   /* return our new path node. */
   return new;
}

int al_uri_path_free (al_uri_path_t *path)
{
   if (path->name)
      free (path->name);
   AL_LL_UNLINK (path, prev, next, path->uri, path);
   free (path);
   return 1;
}

al_uri_parameter_t *al_uri_parameter_append (al_uri_t *uri,
   al_uri_parameter_t *prev, const char *name, const char *value)
{
   /* replace old values with new ones. */
   al_uri_parameter_t *new;
   if ((new = al_uri_parameter_get (uri, name)) != NULL) {
      al_util_replace_string (&(new->value), value);
      return new;
   }

   /* initialize a parameter with our name + value pair. */
   new = calloc (1, sizeof (al_uri_parameter_t));
   new->name  = strdup (name);
   new->value = strdup (value);

   /* link it. */
   new->uri = uri;
   if (prev) {
      prev->next = new;
      new->prev = prev;
   }
   else
      uri->parameters = new;

   /* return our new path node. */
   return new;
}

al_uri_parameter_t *al_uri_parameter_get (al_uri_t *uri, const char *name)
{
   al_uri_parameter_t *p;
   for (p = uri->parameters; p != NULL; p = p->next)
      if (strcmp (p->name, name) == 0)
         return p;
   return NULL;
}

int al_uri_parameter_free (al_uri_parameter_t *param)
{
   if (param->name)
      free (param->name);
   if (param->value)
      free (param->value);
   AL_LL_UNLINK (param, prev, next, param->uri, parameters);
   free (param);
   return 1;
}

al_uri_path_t *al_uri_path_has_v (const al_uri_path_t *path, va_list args)
{
   const char *str;
   const al_uri_path_t *last = NULL;

   /* check 'path' with arguments until the argument is NULL.  for each
    * match, move path to 'path->next'.  if all arguments matched, return
    * the last successful match - otherwise, return NULL. */
   while (1) {
      if ((str = va_arg (args, const char *)) == NULL)
         break;
      if (path == NULL || strcmp (path->name, str) != 0)
         return NULL;
      last = path;
      path = path->next;
   }

   /* return the last path node we found. */
   return (al_uri_path_t *) last;
}

al_uri_path_t *al_uri_path_has (const al_uri_path_t *path, ...)
{
   va_list args;
   al_uri_path_t *result;

   /* send variable arguments to al_uri_path_has_v() and return the result. */
   va_start (args, path);
   result = al_uri_path_has_v (path, args);
   va_end (args);
   return result;
}

al_uri_path_t *al_uri_path_is (const al_uri_path_t *path, ...)
{
   va_list args;
   al_uri_path_t *result;

   /* send variable arguments to al_uri_path_has_v(). */
   va_start (args, path);
   result = al_uri_path_has_v (path, args);
   va_end (args);

   /* if there was no match OR there's additional nodes in the path,
    * return failure.  otherwise, return the successful result. */
   if (result == NULL || result->next != NULL)
      return NULL;
   return result;
}

al_uri_path_t *al_uri_path_at (const al_uri_path_t *path, int index)
{
   while (index > 0 && path) {
      path = path->next;
      index--;
   }
   return (al_uri_path_t *) path;
}

int al_uri_path_length (const al_uri_path_t *path)
{
   int count = 0;
   while (path && path->name[0] != '\0') {
      path++;
      count++;
   }
   return count;
}

char *al_uri_path_full (const al_uri_path_t *path, char *out, size_t size)
{
   const al_uri_path_t *p;
   size_t len = 0;
   out[0] = '\0';
   for (p = path; p != NULL; p = p->next)
      snprintf (out + len, size - len, "%s%s", (path == p &&
         !(path->uri->flags & AL_URI_RELATIVE)) ? "" : "/", path->name);
   return out;
}