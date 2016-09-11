/* modules.c
 * ---------
 * custom data assigned to servers, connections, etc. */

#include <stdlib.h>
#include <string.h>

#include "alpaca/modules.h"

al_module_t *al_module_get (al_module_t *const *list, const char *name)
{
   /* search for a module in 'list' with a matching name. */
   if (list == NULL)
      return NULL;
   al_module_t *m;
   for (m = *list; m != NULL; m = m->next)
      if (strcmp (m->name, name) == 0)
         return m;
   return NULL;
}

al_module_t *al_module_new (void *owner, al_module_t **list, const char *name,
   void *data, size_t data_size, al_module_func *func_free)
{
   /* allocate our new module with basic settings. */
   al_module_t *new = calloc (1, sizeof (al_module_t));
   new->owner     = owner;
   new->name      = strdup (name);
   new->data      = data;
   new->data_size = data_size;
   new->func_free = func_free;

   /* link to the front of 'list'. */
   new->list = list;
   new->next = *list;
   if (new->next)
      new->next->prev = new;
   *list = new;

   /* return our new module. */
   return new;
}

int al_module_free (al_module_t *m)
{
   /* run custom free func. */
   if (m->func_free)
      m->func_free (m, m->data);

   /* unlink from whatever list we're using. */
   if (m->prev) m->prev->next = m->next;
   else         *(m->list)    = m->next;
   if (m->next) m->next->prev = m->prev;

   /* free allocated data. */
   if (m->data)
      free (m->data);

   /* free the structure itself and return success. */
   free (m);
   return 1;
}
