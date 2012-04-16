/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#include "vftasks.h"
#include "platform.h"

#include <stdlib.h>     /* for malloc, free, and abort */
#include <stdio.h>      /* for printing to stderr */

/* Used in structures to enforce entries falling into different cache
   lines (in order to avoid false sharing) */
#define MAX_CACHE_LINE_SIZE 256

/* ***************************************************************************
 * Types
 * ***************************************************************************/

/* forward declaration */

/* vftasks_worker_t is volatile because associated variables can be modified from
   multiple threads; the nonvolatile version of the structure is provided to keep
   pthread and libc library functions happy
 */
typedef volatile struct vftasks_worker_s vftasks_worker_t;
typedef struct vftasks_worker_s vftasks_nv_worker_t;

/** zero or more workers in the pool
 */
typedef struct vftasks_chunk_s
{
  vftasks_worker_t *base;   /* pointer to the first worker in the chunk */
  vftasks_worker_t *limit;  /* pointer to the first byte beyond the last worker in the
                               chunk */
  vftasks_worker_t *next;   /* pointer to the next available worker in the chunk */
} vftasks_chunk_t;

/** worker
 */
struct vftasks_worker_s
{
  /* WARNING: This structure has a very specific layout to enhance cache utilization
     and avoid false sharing, think twice before changing it. */
  /* is_active, busy_wait and task should go in a contiguous memory zone to
     improve cache utilization when spinning */
  int is_active;           /* 0 if inactive, nonzero otherwise */
  int busy_wait;           /* the worker should spin or wait on a semaphore */
  vftasks_task_t *task;    /* task to be executed */
  thread_t thread;         /* pointer to a handle for the thread on which the
                              worker is running */
  tls_key_t key;           /* the TLS-key of the containing pool */

  void *args;              /* task arguments */
  vftasks_chunk_t *chunk;  /* pointer to a chunk of subsidiary workers */
  semaphore_t submit_sem;  /* wait for work semaphore used when busy_wait is 0 */
  semaphore_t get_sem;     /* wait for join semaphore used when busy_wait is 0 */

  /* Avoid false sharing between different tasks */
  char padding[MAX_CACHE_LINE_SIZE];
};

/** worker-thread pool
 */
struct vftasks_pool_s
{
  tls_key_t key; /* TLS-key for the pool */
};

#define WORKER_WAIT(WORKER)                                \
  if ((WORKER)->busy_wait)                                 \
    while ((WORKER)->is_active && (WORKER)->task == NULL); \
  else                                                     \
    SEMAPHORE_WAIT((WORKER)->submit_sem)

#define CALLER_WAIT(WORKER)                                \
  if ((WORKER)->busy_wait)                                 \
    while (worker->task != NULL);                          \
  else                                                     \
    SEMAPHORE_WAIT((WORKER)->get_sem)

#define WORKER_SIGNAL(WORKER)                   \
  if (!(WORKER)->busy_wait)                     \
    SEMAPHORE_POST((WORKER)->submit_sem)

#define CALLER_SIGNAL(WORKER)                   \
  if (!(WORKER)->busy_wait)                     \
    SEMAPHORE_POST((WORKER)->get_sem)

/* ***************************************************************************
 * Aborting on failure
 * ***************************************************************************/

/** abort
 */
static void abort_on_fail(char *msg)
{
#ifdef VFTASKS_ABORT_ON_FAILURE
  fprintf(stderr, "Failure: %s\n", msg);
  abort();
#endif
}

/* ***************************************************************************
 * Workers
 * ***************************************************************************/

/** loop executed by a worker thread
 */
static WORKER_PROTO(vftasks_worker_loop, arg)
{
  vftasks_worker_t *worker;  /* pointer to the worker */

  /* retrieve the worker pointer */
  worker = (vftasks_worker_t *)arg;

  /* store the pointer to the chunk of subsidiary workers in TLS */
  TLS_SET(worker->key, worker->chunk);

  /* worker->is_active is volatile and updated from another thread */
  while (worker->is_active)
  {
    /* wait for work to be submitted */
    WORKER_WAIT(worker);

    /* check whether the worker is still active */
    if (worker->is_active)
    {
      /* execute the assigned task */
      worker->task(worker->args);

      /* forget about the executed task */
      worker->task = NULL;

      /* notify caller that current work has finished */
      CALLER_SIGNAL(worker);
    }
  }

  /* worker is deactivated, so return */
  return THREAD_EXIT_SUCCESS;
}

/* ***************************************************************************
 * Access to thread-local chunks of workers
 * ***************************************************************************/

static inline vftasks_chunk_t *vftasks_get_chunk(vftasks_pool_t *pool)
{
  return (vftasks_chunk_t *)TLS_GET(pool->key);
}

/* ***************************************************************************
 * Creation and destruction of worker-thread pools
 * ***************************************************************************/

static inline int vftasks_initialize_sync(vftasks_worker_t *worker)
{
  if (!worker->busy_wait)
  {
    if ((SEMAPHORE_CREATE(worker->submit_sem, 0, 1)) != 0)
    {
      abort_on_fail("vftasks_create_pool: submit semaphore creation failed");
      return 1;
    }
    if ((SEMAPHORE_CREATE(worker->get_sem, 0, 1)) != 0)
    {
      SEMAPHORE_DESTROY(worker->submit_sem);
      abort_on_fail("vftasks_create_pool: get semaphore creation failed");
      return 1;
    }
  }

  return 0;
}

static inline void vftasks_destroy_sync(vftasks_worker_t *worker)
{
  if (!worker->busy_wait)
  {
    SEMAPHORE_DESTROY(worker->submit_sem);
    SEMAPHORE_DESTROY(worker->get_sem);
  }
}

/** initialize worker
 */
static inline int vftasks_initialize_worker(vftasks_worker_t *worker,
                                            int busy_wait,
                                            tls_key_t key)
{
  /* store the TLS-key for the containing pool */
  worker->key = key;

  /* allocate a chunk of subsidiary workers */
  worker->chunk = (vftasks_chunk_t *)malloc(sizeof(vftasks_chunk_t));
  if (worker->chunk == NULL)
  {
    abort_on_fail("vftasks_create_pool: thread chunk allocation failed");
    return 1;
  }

  /* initially the worker does not have a task assigned */
  worker->task = NULL;

  /* activate the worker and have it running on a freshly forked thread */
  worker->is_active = 1;

  worker->busy_wait = busy_wait;

  if (vftasks_initialize_sync(worker) != 0)
  {
    free(worker->chunk);
    abort_on_fail("vftasks_create_pool: semaphore creation failed");
    return 1;
  }

  if (THREAD_CREATE(worker->thread,
                    vftasks_worker_loop,
                    (vftasks_nv_worker_t *) worker) != 0)
  {
    free(worker->chunk);
    vftasks_destroy_sync(worker);
    abort_on_fail("vftasks_create_pool: thread creation failed");
    return 1;
  }

  /* return 0 to indicate success */
  return 0;
}

/** finalize worker
 */
static inline void vftasks_finalize_worker(vftasks_worker_t *worker)
{
  /* deactivate the worker and join with the thread it is running on */
  worker->is_active = 0;

  /* make sure the worker is not in wait state before joining */
  WORKER_SIGNAL(worker);
  THREAD_JOIN(worker->thread);
  vftasks_destroy_sync(worker);

  /* deallocate the chunk of subsidiary workers */
  free(worker->chunk);
}

/** create and activate a chunk of workers of a given size
 */
static inline vftasks_chunk_t *vftasks_create_workers(int num_workers,
                                                      tls_key_t key,
                                                      int busy_wait)
{
  vftasks_chunk_t *chunk;              /* pointer to the chunk of workers */
  vftasks_worker_t *worker, *worker_;  /* pointers to workers in the chunk */

  if (num_workers <= 0) return NULL;

  /* allocate the chunk */
  chunk = (vftasks_chunk_t *)malloc(sizeof(vftasks_chunk_t));
  if (chunk == NULL) return NULL;

  /* allocate the workers */
  chunk->base = (vftasks_worker_t *) malloc(num_workers * sizeof(vftasks_worker_t));
  if (chunk->base == NULL)
  {
    free(chunk);
    abort_on_fail("vftasks_create_pool: not enough memory");
    return NULL;
  }
  chunk->limit = chunk->base + num_workers;
  chunk->next = chunk->base;

  /* initialize the workers */
  for (worker = chunk->base; worker < chunk->limit; ++worker)
  {
    if (vftasks_initialize_worker(worker, busy_wait, key) != 0)
    {
      /* no get calls have been done at this point so the workers are only waiting
       * on their internal semaphore, which is released by finalize itself.
       */
      for (worker_ = chunk->base; worker_ < worker; ++worker_)
      {
        vftasks_finalize_worker(worker_);
      }
      free((vftasks_nv_worker_t *)chunk->base);
      free(chunk);
      abort_on_fail("vftasks_create_pool: worker initialization failed");
      return NULL;
    }
  }

  /* return the chunk */
  return chunk;
}

/** destroy a chunk of workers
 */
static void vftasks_destroy_workers(vftasks_chunk_t *chunk)
{
  vftasks_worker_t *worker;  /* pointer to a worker in the chunk */

  /* finalize the workers lifo style */
  for (worker = chunk->limit-1; worker >= chunk->base; worker--)
  {
    vftasks_finalize_worker(worker);
  }

  /* deallocate the workers */
  free((vftasks_nv_worker_t *)chunk->base);

  /* deallocate the chunk pointer */
  free(chunk);
}

/** create pool
 */
vftasks_pool_t *vftasks_create_pool(int num_workers, int busy_wait)
{
  vftasks_pool_t *pool;    /* pointer to the pool */
  tls_key_t key;           /* TLS-key for the pool pointer */
  vftasks_chunk_t *chunk;  /* pointer to a chunk containing all the workers */

  /* allocate the pool */
  pool = (vftasks_pool_t *)malloc(sizeof(vftasks_pool_t));
  if (pool == NULL)
  {
    abort_on_fail("vftasks_create_pool: not enough memory");
    return NULL;
  }

  /* create a TLS-key for the pool pointer */
  if (TLS_CREATE(key) != 0)
  {
    free(pool);
    abort_on_fail("vftasks_create_pool: could not create thread local storage");
    return NULL;
  }

  /* store the key with the pool */
  pool->key = key;

  /* create the workers */
  chunk = vftasks_create_workers(num_workers, key, busy_wait);
  if (chunk == NULL)
  {
    TLS_DESTROY(key);
    free(pool);
    abort_on_fail("vftasks_create_pool: worker creation failed");
    return NULL;
  }

  /* store the workers in TLS */
  if (TLS_SET(key, chunk) != 0)
  {
    vftasks_destroy_workers(chunk);
    TLS_DESTROY(key);
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
  TLS_DESTROY(pool->key);

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
  vftasks_worker_t *current;

  if (task == NULL)
  {
    abort_on_fail("vftasks_submit: no task");
    return 1;
  }

  if (num_workers < 0)
  {
    abort_on_fail("vftasks_submit: invalid number of workers");
    return 1;
  }

  /* retrieve the chunk of subsidiary workers */
  chunk = vftasks_get_chunk(pool);
  if (chunk == NULL)
  {
    abort_on_fail("vftasks_submit: no subsidiary worker chunk");
    return 1;
  }

  /* check that there are enough workers available to execute the task */
  if (chunk->next + num_workers >= chunk->limit)
  {
    abort_on_fail("vftasks_submit: insufficient subsidiary workers");
    return 1;
  }

  current = chunk->next;

  /* select the worker that is to execute the task */
  worker = current + num_workers;

  /* assign the worker its subsididary chunk */
  worker->chunk->base = current;
  worker->chunk->limit = current + num_workers;
  worker->chunk->next = current;

  /* update the pointer to the first available worker in this chunk */
  chunk->next = worker + 1;

  /* assign the task and the corresponding arguments to the worker */
  worker->args = args;
  worker->task = task;

  /* signal the (blocked) worker to continue execution */
  WORKER_SIGNAL(worker);

  /* return 0 to indicate success */
  return 0;
}

/** block until the most recently submitted task finishes
 */
int vftasks_get(vftasks_pool_t *pool)
{
  vftasks_chunk_t *chunk;    /* pointer to the chunk of subsidiary workers that the
                                calling thread has at its disposal */
  vftasks_worker_t *worker;  /* pointer to the worker that is executing the task */

  /* retrieve the chunk of subsidiary workers */
  chunk = vftasks_get_chunk(pool);
  if (chunk == NULL)
  {
    abort_on_fail("vftasks_get: no subsidiary worker chunk");
    return 1;
  }

  /* check that there is a task being executed */
  if (chunk->next <= chunk->base)
  {
    abort_on_fail("vftasks_get: no executing task");
    return 1;
  }

  /* retrieve the worker that is executing most recently submitted task in this chunk */
  worker = chunk->next - 1;

  /* wait until the task has finished execution */
  CALLER_WAIT(worker);

  /* release the current worker and its subsidiary chunk
   * it is assumed that all subsidiary workers have joined
   */
  chunk->next = worker->chunk->base;

  /* return 0 to indicate success */
  return 0;
}
