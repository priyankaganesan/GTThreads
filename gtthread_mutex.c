/**********************************************************************
gtthread_mutex.c.  

This file contains the implementation of the mutex subset of the
gtthreads library.  The locks can be implemented with a simple queue.
 **********************************************************************/

/*
  Include as needed
*/

#include "gtthread.h"

/*
  The gtthread_mutex_init() function is analogous to
  pthread_mutex_init with the default parameters enforced.
  There is no need to create a static initializer analogous to
  PTHREAD_MUTEX_INITIALIZER.
 */
int gtthread_mutex_init(gtthread_mutex_t* mutex)
{
  //Error if initializing already initialized mutex
  if((mutex->init) == 1){
    return EBUSY;
  }
  //Normal mutex init leaves mutex unlocked (0), sets init to 1
  mutex->init = 1;
  mutex->set = 0;
  mutex->id = -1;
  return 0;
}

/*
  The gtthread_mutex_lock() is analogous to pthread_mutex_lock.
  Returns zero on success.
 */
int gtthread_mutex_lock(gtthread_mutex_t* mutex)
{
  //Uninitialized mutex
  if((mutex->init) == 0){
    return EINVAL;
  }
  //Check if mutex is already locked by the calling thread
  if (gtthread_equal(gtthread_self(), mutex->id)) {
    return EDEADLK;
  }

  block_signal();
  //Check if mutex is locked, but by another thread. Wait for release.
  while ((mutex->set) == 1){
    gtthread_yield();
  }
  //Lock the mutex
  mutex->set = 1;
  mutex->id = gtthread_self();

  unblock_signal();
  return 0;
}

/*
  The gtthread_mutex_unlock() is analogous to pthread_mutex_unlock.
  Returns zero on success.
 */
int gtthread_mutex_unlock(gtthread_mutex_t *mutex)
{
  //Uninitialized mutex
  if((mutex->init) == 0){
    return EINVAL;
  }
  //The current thread does not own the mutex
  if (!gtthread_equal(gtthread_self(), mutex->id)) {
    return EPERM;
  }
  block_signal();
  //Unlock the mutex
  mutex->set = 0;
  mutex->id = -1;

  unblock_signal();
  return 0;
}

/*
  The gtthread_mutex_destroy() function is analogous to
  pthread_mutex_destroy and frees any resources associated with the mutex.
*/
int gtthread_mutex_destroy(gtthread_mutex_t *mutex)
{
  //Error if destroying locked mutex
  if((mutex->set) == 1){
    return EBUSY;
  }
  //Error if destroying already destroyed mutex
  else if((mutex->init) == 0){
    return EINVAL;
  }
  //Normal destroy sets 'init' to 0;
  mutex->init = 0;
  mutex->id = -1;
  return 0;
}
