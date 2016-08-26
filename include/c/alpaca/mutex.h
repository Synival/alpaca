/* mutex.h
 * -------
 * simple wrapper around pthread mutexes for more convenient functionality. */

#ifndef __ALPACA_C_MUTEX_H
#define __ALPACA_C_MUTEX_H

#include <pthread.h>

#include "defs.h"

/* simple pthread mutex wrapper. */
struct _al_mutex_t {
   int locks;
   pthread_t p_thread;
   pthread_mutex_t p_mutex;
};

/* handy mutex functions. */
al_mutex_t *al_mutex_new (void);
int al_mutex_free (al_mutex_t *mutex);
int al_mutex_lock (al_mutex_t *mutex);
int al_mutex_unlock (al_mutex_t *mutex);

#endif
