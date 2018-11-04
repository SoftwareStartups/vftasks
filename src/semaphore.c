#include "semaphore.h"

#ifdef _POSIX_SOURCE

int _vftasks_sem_create(_vftasks_semaphore_t *sem, int value)
{
  int r;

  sem->value = value;
  r = pthread_mutex_init(&sem->lock, NULL);
  if (!r) r = pthread_cond_init(&sem->flag, NULL);

  return (r != 0);
}

int _vftasks_sem_destroy(_vftasks_semaphore_t *sem)
{
  int r, s;

  r = pthread_cond_destroy(&sem->flag);
  s = pthread_mutex_destroy(&sem->lock);

  return (r || s);
}

int _vftasks_sem_wait(_vftasks_semaphore_t *sem)
{
  int r, s;

  r = pthread_mutex_lock(&sem->lock);
  s = 0;

  if (!r)
  {
    s = (--sem->value) < 0 ? pthread_cond_wait(&sem->flag, &sem->lock) : 0;
    r = pthread_mutex_unlock(&sem->lock);
  }

  return (r || s);
}


int _vftasks_sem_post(_vftasks_semaphore_t *sem)
{
  int r, s;

  r = pthread_mutex_lock(&sem->lock);
  s = 0;

  if (!r)
  {
    sem->value++;
    s = pthread_cond_signal(&sem->flag);
    r = pthread_mutex_unlock(&sem->lock);
  }

  return (r || s);
}

#endif /* _POSIX_SOURCE */
