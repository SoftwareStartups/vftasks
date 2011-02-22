/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#ifndef __VFTASKS_H
#define __VFTASKS_H

#include <stddef.h>    /* for NULL */

/* ***************************************************************************
 * Types
 * ***************************************************************************/

/** Represents a worker-thread pool.
 */
typedef struct vftasks_pool_s vftasks_pool_t;

/** Represents a task that is to be executed in the worker-thread pool.
 */
typedef void *(vftasks_task_t)(void *);

/* ***************************************************************************
 * Creation and destruction of worker-thread pools
 * ***************************************************************************/

/** Creates a worker-thread pool of a given size.
 *
 *  @param  num_workers  The number of worker threads in the pool.
 *
 *  @return
 *    On success, a pointer to the pool.
 *    On failure, NULL.
 */
vftasks_pool_t *vftasks_create_pool(int num_workers);

/** Destroys a given worker-thread pool.
 *
 *  @param  pool  A pointer to the pool.
 */
void vftasks_destroy_pool(vftasks_pool_t *pool);

/* ***************************************************************************
 * Execution of parallel tasks
 * ***************************************************************************/

/** Submits a specified instance of a task to a given worker-thread pool.
 *
 *  Executes the task in parallel on a thread from the pool.
 *
 *  @param  pool         A pointer to the pool.
 *  @param  task         A pointer to the task.
 *  @param  args         A pointer to the arguments for the instance.
 *  @param  num_workers  The number of additional worker threads that will be required
 *                       for the execution of the task.
 *
 *  @return
 *    On success, 0
 *    On failure, a nonzero value.
 */
int vftasks_submit(vftasks_pool_t *pool,
                   vftasks_task_t *task,
                   void *args,
                   int num_workers);

/** Retrieves the result of a task that is being executed in a given worker-thread pool.
 *
 *  Blocks until execution of the task has completed and then stores the result in the
 *  location that is referenced by result.
 *
 *  @param  pool    A pointer to the pool.
 *  @param  result  A reference to the location into which the result is to be stored.
 *                  If result is NULL, the result is not stored.
 *
 *  @return
 *    On success, 0.
 *    On failure, a nonzero value.
 */
int vftasks_get(vftasks_pool_t *pool, void **result);

#endif /* __VFTASKS_H */
