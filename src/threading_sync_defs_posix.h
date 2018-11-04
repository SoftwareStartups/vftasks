#ifndef THREADING_SYNC_DEFS_POSIX_H
#define THREADING_SYNC_DEFS_POSIX_H

#include <pthread.h>
#include "semaphore.h"


typedef pthread_t thread_t;
typedef pthread_key_t tls_key_t;
typedef pthread_mutex_t mutex_t;
typedef _vftasks_semaphore_t semaphore_t;


#define THREAD_CREATE(THREAD,FUNC,ARGS) \
  pthread_create((pthread_t *)&(THREAD), NULL, FUNC, ARGS)

#define THREAD_EXIT() pthread_exit(NULL)
#define THREAD_JOIN(THREAD) pthread_join(THREAD, NULL)

#define TLS_CREATE(KEY) pthread_key_create(&(KEY), NULL)
#define TLS_DESTROY(KEY) pthread_key_delete(KEY)
#define TLS_SET(KEY,VAL) pthread_setspecific(KEY, VAL)
#define TLS_GET(KEY) pthread_getspecific(KEY)


#define WORKER_PROTO(FUNC,ARG) void *FUNC(void *ARG)
#define THREAD_EXIT_SUCCESS NULL


#define MUTEX_LOCK(MUTEX) pthread_mutex_lock(&(MUTEX))
#define MUTEX_UNLOCK(MUTEX) pthread_mutex_unlock(&(MUTEX))
#define MUTEX_CREATE(MUTEX) pthread_mutex_init(&(MUTEX), NULL)

#define MUTEX_DESTROY(MUTEX)                    \
  {                                             \
    MUTEX_UNLOCK(MUTEX);                        \
    pthread_mutex_destroy(&(MUTEX));            \
  }


#define SEMAPHORE_CREATE(SEM,VALUE,MAX) \
  _vftasks_sem_create((_vftasks_semaphore_t *)(&(SEM)), VALUE)
#define SEMAPHORE_DESTROY(SEM) _vftasks_sem_destroy((_vftasks_semaphore_t *)(&(SEM)))
#define SEMAPHORE_WAIT(SEM) _vftasks_sem_wait((_vftasks_semaphore_t *)(&(SEM)))
#define SEMAPHORE_POST(SEM) _vftasks_sem_post((_vftasks_semaphore_t *)(&(SEM)))


#endif /* THREADING_SYNC_DEFS_POSIX_H */
