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

/* ***************************************************************************
 * Two-dimensional synchronization between tasks
 * ***************************************************************************/

/** A handle that is to be used to manage two-dimensional synchronization between
 *  concurrent tasks.
 */
typedef struct vfsync_2d_mgr_s vfsync_2d_mgr_t;

/** Creates a handle for managing two-dimensional synchronization between concurrent
 *  tasks.
 *
 *  @param dim_x   The size of the first dimension of the joint iteration space of the
 *                 concurrent tasks.
 *  @param dim_y   The size of the second dimension of the joint iteration space of the
 *                 concurrent tasks.
 *  @param dist_x  The critical dependency distance along the first dimension of the
 *                 joint iteration space of the concurrent tasks.
 *  @param dist_y  The ciritial dependency distance along the second dimention of the
 *                 joint iteration space of the concurrent tasks.
 *
 *  @return
 *    On success, a pointer to the handle.
 *    On failure, NULL.
 */
vfsync_2d_mgr_t *vfsync_create_2d_mgr(int dim_x, int dim_y, int dist_x, int dist_y);

/** Destroys a given handle for managing two-dimension synchronization between
 *  concurrent tasks.
 *
 *  @mgr  A pointer to the handle.
 */
void vfsync_destroy_2d_mgr(vfsync_2d_mgr_t *mgr);

/** Signals the completion of an inner iteration through a handle for managing
 *  two-dimensional synchronization between concurrent task.
 *
 *  @param mgr  A pointer to the handle.
 *  @param x    The iteration's first-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *  @param y    The iteration's second-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *
 *  @return
 *    On success, 0.
 *    On failure, a nonzero value.
 */
int vfsync_sigal_2d(vfsync_2d_mgr_t *mgr, int x, int y);

/** Synchronizes a task at the start of an inner iteration with the tasks it is
 *  depending on.
 *
 *  @param mgr  A pointer to the handle that manages synchronization.
 *  @param x    The iteration's first-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *  @param y    The iteration's second-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 */
int vfsync_wait_2d(vfsync_2d_mgr_t *mgr, int x, int y);

#endif /* __VFTASKS_H */
