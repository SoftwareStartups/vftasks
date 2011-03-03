/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#include "vftasks.h"

#include <stdlib.h>   /* for malloc, free */
#include <pthread.h>  /* POSIX Threads API */

/* not all compilers recognize __inline__ */
#ifndef __GNUC__
#define __inline__
#endif

/* ***************************************************************************
 * Types
 * ***************************************************************************/

/* forward declaration */

/* vftasks_worker_t is volatile because associated variables can
   be modified from multiple threads. The non-volatile version of
   the structure is provided to keep pthread and libc library functions happy */
typedef volatile struct vftasks_worker_s vftasks_worker_t;
typedef struct vftasks_worker_s vftasks_nv_worker_t;

/** zero or more workers in the pool
 */
typedef struct vftasks_chunk_s
{
  vftasks_worker_t *base;   /* pointer to the first worker in the chunk */
  vftasks_worker_t *limit;  /* pointer to the first byte beyond the last worker in the
                               chunk */
  vftasks_worker_t *head;   /* pointer to the worker in the chunk that is executing the
                               oldest task */
  vftasks_worker_t *tail;   /* pointer to the next available worker in the chunk */
} vftasks_chunk_t;

/** worker
 */
struct vftasks_worker_s
{
  int is_active;           /* 0 if inactive, nonzero otherwise */
  pthread_t *thread;       /* pointer to a handle for the POSIX thread on which the
                              worker is running */
  pthread_key_t key;       /* the TLS-key of the containing pool */
  vftasks_chunk_t *chunk;  /* pointer to a chunk of subsidiary workers */
  vftasks_task_t *task;    /* task to be executed */
  void *args;              /* task arguments */
  void *result;            /* result of the task */
};

/** worker-thread pool
 */
struct vftasks_pool_s
{
  pthread_key_t key;  /* TLS-key for the pool */
};

/* ***************************************************************************
 * Workers
 * ***************************************************************************/

/** loop executed by a worker thread
 */
void *vftasks_worker_loop(void *args)
{
  vftasks_worker_t *worker;  /* pointer to the worker */

  /* retrieve the worker pointer */
  worker = (vftasks_worker_t *)args;

  /* store the pointer to the chunk of subsidiary workers in TLS */
  pthread_setspecific(worker->key, worker->chunk);

  /* execute the worker loop for as long as the worker is active */
  while (worker->is_active)
  {
    /* spin until a task is assigned to the worker */
    while (worker->is_active && worker->task == NULL)
    {      
    }

    /* check whether the worker is still active */
    if (worker->is_active)
    {
      /* execute the assigned task and store the result */
      worker->result = worker->task(worker->args);

      /* forget about the executed task*/
      worker->task = NULL;
    }
  }

  /* worker is deactivated, so return */
  return NULL;
}

/* ***************************************************************************
 * Access to thread-local chunks of workers
 * ***************************************************************************/

__inline__ vftasks_chunk_t *vftasks_get_chunk(vftasks_pool_t *pool)
{
  return (vftasks_chunk_t *)pthread_getspecific(pool->key);
}

/* ***************************************************************************
 * Creation and destruction of worker-thread pools
 * ***************************************************************************/

/** initialize worker
 */
  __inline__ int vftasks_initialize_worker(vftasks_worker_t *worker, pthread_key_t key)
{
  int rc;

  /* allocate a handle to a POSIX thread */
  worker->thread = (pthread_t *)malloc(sizeof(pthread_t));
  if (worker->thread == NULL)
  {
    return 1;
  }

  /* store the TLS-key for the containing pool */
  worker->key = key;

  /* allocate a chunk of subsidiary workers */
  worker->chunk = (vftasks_chunk_t *)malloc(sizeof(vftasks_chunk_t));
  if (worker->chunk == NULL)
  {
    free(worker->thread);
    return 1;
  }

  /* activate the worker and have it running on a freshly forked thread */
  worker->is_active = 1;
  rc = pthread_create(worker->thread,
                      NULL,
                      vftasks_worker_loop,
                      (vftasks_nv_worker_t *) worker);
  if (rc != 0)
  {
    free(worker->chunk);
    free(worker->thread);
    return 1;
  }

  /* return 0 to indicate success */
  return 0;
}

/** finalize worker
 */
  __inline__ void vftasks_finalize_worker(vftasks_worker_t *worker)
{
  /* deactivate the worker and join with the thread it is running on */
  worker->is_active = 0;
  pthread_join(*worker->thread, NULL);

  /* deallocate the chunk of subsidiary workers */
  free(worker->chunk);

  /* deallocate the handle to the thread */
  free(worker->thread);
}

/** create and activate a chunk of workers of a given size
 */
  __inline__ vftasks_chunk_t *vftasks_create_workers(int num_workers, pthread_key_t key)
{
  vftasks_chunk_t *chunk;              /* pointer to the chunk of workers */
  vftasks_worker_t *worker, *worker_;  /* pointers to workers in the chunk */

  /* allocate the chunk */
  chunk = (vftasks_chunk_t *)malloc(sizeof(vftasks_chunk_t));
  if (chunk == NULL) return NULL;

  /* allocate the workers */
  chunk->base = (vftasks_worker_t *) malloc(num_workers * sizeof(vftasks_worker_t));
  if (chunk->base == NULL)
  {
    free(chunk);
    return NULL;
  }
  chunk->limit = chunk->base + num_workers;
  chunk->head = chunk->base;
  chunk->tail = chunk->base;

  /* initialize the workers */
  for (worker = chunk->base; worker < chunk->limit; ++worker)
  {
    if (vftasks_initialize_worker(worker, key) != 0)
    {
      for (worker_ = chunk->base; worker_ < worker; ++worker_)
      {
        vftasks_finalize_worker(worker_);
      }
      free((vftasks_nv_worker_t *)chunk->base);
      free(chunk);
      return NULL;
    }
  }

  return chunk;
}

/** destroy a chunk of workers
 */
vftasks_destroy_workers(vftasks_chunk_t *chunk)
{
  vftasks_worker_t *worker;  /* pointer to a worker in the chunk */

  /* finalize the workers */
  for (worker = chunk->base; worker < chunk->limit; ++worker)
  {
    vftasks_finalize_worker(worker);
  }

  /* deallocate the workers */
  free((vftasks_nv_worker_t *)chunk->base);

  /* deallocate the chunk */
  free(chunk);
}


/** create pool
 */
vftasks_pool_t *vftasks_create_pool(int num_workers)
{
  vftasks_pool_t *pool;    /* pointer to the pool */
  pthread_key_t key;       /* TLS-key for the pool pointer */
  vftasks_chunk_t *chunk;  /* pointer to a chunk containing all the workers */

  /* allocate the pool */
  pool = (vftasks_pool_t *)malloc(sizeof(vftasks_pool_t));
  if (pool == NULL) return NULL;

  /* create a TLS-key for the pool pointer */
  if (pthread_key_create(&key, NULL) != 0)
  {
    free(pool);
    return NULL;
  }

  /* store the key with the pool */
  pool->key = key;

  /* create the workers */
  chunk = vftasks_create_workers(num_workers, key);
  if (chunk == NULL)
  {
    pthread_key_delete(key);
    free(pool);
    return NULL;    
  }

  /* store the workers in TLS */
  if (pthread_setspecific(key, chunk) != 0)
  {
    vftasks_destroy_workers(chunk);
    pthread_key_delete(key);
    free(pool);
    return NULL;
  }

  /* return the pool pointer */
  return pool;
}

/** destroy pool
 */
void vftasks_destroy_pool(vftasks_pool_t *pool)
{
  /* retrieve the chunk that contains the workers in the pool and destroy the workers */
  vftasks_destroy_workers(vftasks_get_chunk(pool));  

  /* delete the TLS-key for the pool */
  pthread_key_delete(pool->key);

  /* deallocate the pool */
  free(pool);
}

/* ***************************************************************************
 * Execution of parallel tasks
 * ***************************************************************************/

/** submit a task
 */
int vftasks_submit(vftasks_pool_t *pool,
                   vftasks_task_t *task,
                   void *args,
                   int num_workers)
{
  vftasks_chunk_t *chunk;    /* pointer to the chunk of subsidiary workers that the
                                calling thread has at its disposal */
  vftasks_worker_t *worker;  /* pointer to the worker that is to execute the task */

  /* retrieve the chuck of subsidiary workers */
  chunk = vftasks_get_chunk(pool);
  if (chunk == NULL) return 1;

  /* check that there are enough workers available to execute the task */
  if (chunk->tail + num_workers >= chunk->limit)
  {
    return 1;
  }

  /* select the worker that is to execute the task */
  worker = chunk->tail;

  /* assign the worker its subsididary workers */
  worker->chunk->base = worker + 1;
  worker->chunk->limit = worker + num_workers + 1;
  worker->chunk->head = worker + 1;
  worker->chunk->tail = worker + 1;

  /* update the chunk of subsidiary workers of the calling thread */
  chunk->tail = worker + num_workers + 1;

  /* assign the task and the corresponding arguments to the worker */
  worker->args = args;
  worker->task = task;

  /* return 0 to indicate success */
  return 0;
}

/** retrieve the result of an executed task
 */
int vftasks_get(vftasks_pool_t *pool, void **result)
{
  vftasks_chunk_t *chunk;    /* pointer to the chunk of subsidiary workers that the
                                calling thread has at its disposal */
  vftasks_worker_t *worker;  /* pointer to the worker that is executing the task */

  /* retrieve the chuck of subsidiary workers */
  chunk = vftasks_get_chunk(pool);
  if (chunk == NULL) return 1;

  /* check that there is a task being executed */
  if (chunk->head >= chunk->tail) return 1;

  /* retrieve the worker that is executing the oldest task */
  worker = chunk->head;

  /* spin until the worker has completed the task */
  while (worker->task != NULL)
  {
  }

  /* store the result */
  if (result != NULL) *result = worker->result;

  /* update the chunk of subsidiary workers of the calling thread */
  chunk->head = worker->chunk->limit;

  /* if the all outstanding tasks have been executed, reset the chunk of subsidiary
     workers */
  if (chunk->head == chunk->tail)
  {
    chunk->head = chunk->base;
    chunk->tail = chunk->base;
  }

  /* return 0 to indicate success */
  return 0;
}
