/* Copyright (c) 2012 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

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
