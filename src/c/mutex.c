/* mutex.c
 * -------
 * simple wrapper around pthread mutexes for more convenient functionality. */

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "alpaca/mutex.h"

al_mutex_t *al_mutex_new (void)
{
   /* allocate a new mutex wrapper. */
   al_mutex_t *new = calloc (1, sizeof (al_mutex_t));

   /* create a pthread_mutex_t. */
   pthread_mutexattr_t p_attr;
   pthread_mutexattr_init (&p_attr);
#ifdef _GNU_SOURCE
   pthread_mutexattr_settype (&p_attr, PTHREAD_MUTEX_RECURSIVE);
#endif
   pthread_mutex_init (&(new->p_mutex), &p_attr);

   /* return our new thread wrapper. */
   return new;
}

int al_mutex_free (al_mutex_t *mutex)
{
   /* destroy pthread mutex deallocate our wrapper. */
   pthread_mutex_destroy (&(mutex->p_mutex));
   free (mutex);
   return 1;
}

int al_mutex_lock (al_mutex_t *mutex)
{
#ifdef _GNU_SOURCE
   return pthread_mutex_lock (&(mutex->p_mutex));
#else
   /* TODO: ironically, this function isn't thread-safe. fix! */
   pthread_t self = pthread_self ();
   int r;

   /* if we own this thread and we've already locked it,
    * increment our lock counter. */
   if (mutex->locks > 0 && mutex->p_thread == self) {
      r = 0;
      mutex->locks++;
   }
   /* otherwise, attempt to lock normally and claim ownership. */
   else {
      if ((r = pthread_mutex_lock (&(mutex->p_mutex))) != 0)
         return r;
      mutex->locks++;
      mutex->p_thread = self;
   }
   return r;
#endif
}

int al_mutex_unlock (al_mutex_t *mutex)
{
#ifdef _GNU_SOURCE
   return pthread_mutex_unlock (&(mutex->p_mutex));
#else
   /* TODO: ironically, this function isn't thread-safe. fix! */
   pthread_t self = pthread_self ();

   /* if it's not locked, return an error. */
   if (mutex->locks == 0)
      return EPERM;
   /* if we own this lock and there are recursive locks,
    * decrement the lock counter and return success. */
   if (mutex->p_thread == self && mutex->locks > 1) {
      mutex->locks--;
      return 0;
   }
   /* otherwise, attempt to lock it normally. */
   else {
      int r = pthread_mutex_unlock (&(mutex->p_mutex));
      if (r == 0)
         mutex->locks--;
      return r;
   }
#endif
}
