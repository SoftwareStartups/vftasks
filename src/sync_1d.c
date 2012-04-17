/* Copyright (c) 2012 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#include "vftasks.h"
#include "platform.h"

#include <stdlib.h>     /* abort */
#include <stdio.h>      /* for printing to stderr */

/* ***************************************************************************
 * One-dimensional synchronization between tasks
 * ***************************************************************************/

/** 1D-synchronization manager
 */
struct vftasks_1d_sync_mgr_s
{
  int num_threads;    /* number of threads */
  int dist;           /* critical distance */
  semaphore_t *sems;  /* pointer to an array of num_threads semaphores */
};

/** abort
 */
void _vftasks_abort_on_fail_sync_1d(char *msg)
{
#ifdef VFTASKS__VFTASKS_ABORT_ON_FAIL_SYNC_1DURE
  fprintf(stderr, "Failure: %s\n", msg);
  abort();
#endif
}

/** create a 1D-synchronization manager
 */
vftasks_1d_sync_mgr_t *vftasks_create_1d_sync_mgr(int num_threads, int dist)
{
  vftasks_1d_sync_mgr_t *mgr;  /* pointer to the manager */
  int t;                       /* index */

  /* allocate a manager */
  mgr = (vftasks_1d_sync_mgr_t *)malloc(sizeof(vftasks_1d_sync_mgr_t));
  if (mgr == NULL)
  {
    _vftasks_abort_on_fail_sync_1d("vftasks_create_1d_mgr: not enough memory");
    return NULL;
  }

  /* set the data for manager */
  mgr->num_threads = num_threads;
  mgr->dist = dist;

  /* allocate the semaphores held by the manager */
  mgr->sems = (semaphore_t *)malloc(num_threads * sizeof(semaphore_t));
  if (mgr->sems == NULL)
  {
    free(mgr);
    _vftasks_abort_on_fail_sync_1d("vftasks_create_1d_mgr: not enough memory");
    return NULL;
  }

  /* initialize the semaphores */
  for (t = 0; t < num_threads; ++t)
  {

    SEMAPHORE_CREATE(mgr->sems[t],
                     dist / num_threads + (t < dist % num_threads),
                     dist / num_threads + 1);
  }

  /* return the pointer to the manager */
  return mgr;
}

/** destroy a 1D-synchronization manager
 */
void vftasks_destroy_1d_sync_mgr(vftasks_1d_sync_mgr_t *mgr)
{
  int t;  /* index */

  /* destroy the semaphores held by the manager */
  for (t = 0; t < mgr->num_threads; ++t)
  {
    SEMAPHORE_DESTROY(mgr->sems[t]);
  }

  /* deallocate the semaphores */
  free(mgr->sems);

  /* deallocate the manager */
  free(mgr);
}

/** signal production of data
 */
int vftasks_signal_1d(vftasks_1d_sync_mgr_t *mgr, int i)
{
  int t;  /* index of the thread to signal to */

  /* determing the index of the thread to signal to */
  t = (i + mgr->dist) % mgr->num_threads;

  /* signal through the thread's semaphore; on failure, return 1 */
  if (SEMAPHORE_POST(mgr->sems[t]) != 0)
  {
    _vftasks_abort_on_fail_sync_1d("vftasks_signal_1d");
    return 1;
  }

  /* return 0 to indicate success */
  return 0;
}

/** synchronize before consuming data
 */
int vftasks_wait_1d(vftasks_1d_sync_mgr_t *mgr, int i)
{
  int t; /* index of the executing thread */

  /* determine the index of the executing thread */
  t = (i % mgr->num_threads);

  /* wait through the other thread's semaphore; on failure, return 1 */
  if (SEMAPHORE_WAIT(mgr->sems[t]) != 0)
  {
    _vftasks_abort_on_fail_sync_1d("vftasks_wait_1d");
    return 1;
  }

  /* return 0 to indicate success */
  return 0;
}
