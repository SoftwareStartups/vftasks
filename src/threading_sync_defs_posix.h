/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#ifndef THREADING_SYNC_DEFS_POSIX_H
#define THREADING_SYNC_DEFS_POSIX_H

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


typedef pthread_t*  thread_t;
typedef pthread_key_t tls_key_t;
typedef pthread_mutex_t mutex_t;
typedef sem_t semaphore_t;


#define THREAD_CREATE(THREAD,FUNC,ARGS)               \
  ((THREAD) = (pthread_t *)malloc(sizeof(pthread_t)), \
   pthread_create((THREAD), NULL, FUNC, (ARGS)))

#define THREAD_DESTROY(THREAD) \
  {                            \
    if (THREAD != NULL)        \
      free(THREAD);            \
  }

#define THREAD_JOIN(THREAD)                     \
  {                                             \
    pthread_join(*(THREAD), NULL);              \
    THREAD_DESTROY(THREAD);                     \
  }

#define THREAD_EXIT pthread_exit


#define TLS_CREATE(KEY) pthread_key_create(&(KEY), NULL)
#define TLS_DESTROY(KEY) pthread_key_delete(KEY)
#define TLS_SET(KEY,VAL) pthread_setspecific(KEY, VAL)
#define TLS_GET(KEY) pthread_getspecific(KEY)


#define SLEEP(USECS) usleep(USECS * 10)
#define WORKER_PROTO(FUNC,ARG) void *FUNC(void *ARG)
#define THREAD_EXIT_SUCCESS NULL


#define MUTEX_LOCK(MUTEX) pthread_mutex_lock(&(MUTEX))
#define MUTEX_UNLOCK(MUTEX) pthread_mutex_unlock(&(MUTEX))
#define MUTEX_CREATE(MUTEX) pthread_mutex_init(&(MUTEX), NULL)
#define MUTEX_DESTROY(MUTEX) pthread_mutex_destroy(&(MUTEX))


#define SEMAPHORE_CREATE(SEM,MAX) sem_init(&(SEM), 0, 0);
#define SEMAPHORE_DESTROY(SEM) sem_destroy(&(SEM))
#define SEMAPHORE_WAIT(SEM) sem_wait(&(SEM))
#define SEMAPHORE_POST(SEM) sem_post(&(SEM))

#endif /* THREADING_SYNC_DEFS_POSIX_H */