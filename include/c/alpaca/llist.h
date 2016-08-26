/* llist.h
 * -------
 * common linked-list macros. */

#ifndef __ALPACA_C_LLIST_H
#define __ALPACA_C_LLIST_H

/* global linked-list macros. */
#define LLIST_LINK_FRONT_GLOBAL(NODE, PREV, NEXT, PARENT_LIST) \
   do { \
      NODE->NEXT = PARENT_LIST; \
      NODE->PREV = NULL; \
      PARENT_LIST = NODE; \
      if (NODE->NEXT) \
         NODE->NEXT->PREV = NODE; \
   } while (0)

#define LLIST_LINK_BACK_GLOBAL(TYPE, NODE, PREV, NEXT, PARENT_LIST) \
   do { \
      NODE->NEXT = NULL; \
      if (PARENT_LIST == NULL) { \
         PARENT_LIST = NODE; \
         NODE->PREV = NULL; \
      } \
      else { \
         TYPE *n; \
         for (n = PARENT_LIST; n->NEXT != NULL; n = n->NEXT); \
         n->NEXT = NODE; \
         NODE->PREV = n; \
      } \
   } while (0)

#define LLIST_UNLINK_GLOBAL(NODE, PREV, NEXT, PARENT_LIST) \
   do { \
      if (NODE->PREV) \
         NODE->PREV->NEXT = NODE->NEXT; \
      else \
         PARENT_LIST = NODE->NEXT; \
      if (NODE->NEXT) \
         NODE->NEXT->PREV = NODE->PREV; \
      NODE->NEXT = NULL; \
      NODE->PREV = NULL; \
   } while (0)

/* non-global linked-list macros. */
#define LLIST_LINK_FRONT(NODE, UP, PREV, NEXT, PARENT, LIST) \
   do { \
      NODE->UP = PARENT; \
      NODE->NEXT = PARENT->LIST; \
      NODE->PREV = NULL; \
      PARENT->LIST = NODE; \
      if (NODE->NEXT) \
         NODE->NEXT->PREV = NODE; \
   } while (0)

#define LLIST_LINK_BACK(TYPE, NODE, UP, PREV, NEXT, PARENT, LIST) \
   do { \
      NODE->UP = PARENT; \
      NODE->NEXT = NULL; \
      if (PARENT->LIST == NULL) { \
         PARENT->LIST = NODE; \
         NODE->PREV = NULL; \
      } \
      else { \
         TYPE *n; \
         for (n = PARENT->LIST; n->NEXT != NULL; n = n->NEXT); \
         n->NEXT = NODE; \
         NODE->PREV = n; \
      } \
   } while (0)

#define LLIST_LINK_AFTER(NODE, UP, PREV, NEXT, PARENT, LIST, NEW_PREV) \
   do { \
      NODE->UP = PARENT; \
      NODE->PREV = NEW_PREV; \
      if (NEW_PREV) { \
         NODE->NEXT = NEW_PREV->NEXT; \
         NEW_PREV->NEXT = NODE; \
      } \
      else { \
         NODE->NEXT = PARENT->LIST; \
         PARENT->LIST = NODE; \
      } \
      if (NODE->NEXT) \
         NODE->NEXT->PREV = NODE; \
   } while (0)

#define LLIST_UNLINK(NODE, PREV, NEXT, PARENT, LIST) \
   do { \
      if (NODE->PREV) \
         NODE->PREV->NEXT = NODE->NEXT; \
      else \
         PARENT->LIST = NODE->NEXT; \
      if (NODE->NEXT) \
         NODE->NEXT->PREV = NODE->PREV; \
      PARENT     = NULL; \
      NODE->NEXT = NULL; \
      NODE->PREV = NULL; \
   } while (0)

#endif
