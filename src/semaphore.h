#ifndef __SEMAPHORE_H

#include "vftasks.h"

#ifdef _POSIX_SOURCE

#include <pthread.h>

typedef struct
{
  int value;
  pthread_mutex_t lock;
  pthread_cond_t flag;
} _vftasks_semaphore_t;

int _vftasks_sem_create(_vftasks_semaphore_t *, int);
int _vftasks_sem_destroy(_vftasks_semaphore_t *);
int _vftasks_sem_wait(_vftasks_semaphore_t *);
int _vftasks_sem_post(_vftasks_semaphore_t *);

#endif /* _POSIX_SOURCE */
#endif /* __SEMAPHORE_H */
