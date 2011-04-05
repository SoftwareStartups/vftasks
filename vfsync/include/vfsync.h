/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#ifndef __VFSYNC_H
#define __VFSYNC_H

/** \mainpage vfSync API documentation
 *
 * \section sec_error_handling Error handling
 *  By default, members of the vfSync API terminate the calling program when a failure
 *  is encountered.
 *  This behaviour can be overriden by compiling vfSync with the
 *  VFSYNC_ABORT_ON_FAILURE preprocessor symbols undefined.
 */

#include <stddef.h>    /* for NULL */

#define VFSYNC_ABORT_ON_FAILURE

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
 *
 *  NOTE: If vfSync was compiled with the VFSYNC_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
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
 *
 *  NOTE: If vfSync was compiled with the VFSYNC_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
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
 *
 *  @return
 *    On success, 0.
 *    On failure, a nonzero value.
 *
 *  NOTE: If vfSync was compiled with the VFSYNC_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
int vfsync_wait_2d(vfsync_2d_mgr_t *mgr, int x, int y);

#endif /* __VFSYNC_H */
