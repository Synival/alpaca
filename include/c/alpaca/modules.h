/* modules.h
 * ---------
 * custom data assigned to servers, connections, etc. */

#ifndef __ALPACA_C_MODULES_H
#define __ALPACA_C_MODULES_H

#include "defs.h"

/* our generic structure, designed to work with anything. */
struct _al_module_t {
   /* properties. */
   char *name;
   void *data;
   size_t data_size;
   al_module_func *func_free;

   /* generic list management. */
   void *owner;
   al_module_t **list, *prev, *next;
};

/* functions for module management. */
al_module_t *al_module_get (al_module_t **list, char *name);
al_module_t * al_module_new (void *owner, al_module_t **list, char *name,
   void *data, size_t data_size, al_module_func *func_free);
int al_module_free (al_module_t *m);

#endif
