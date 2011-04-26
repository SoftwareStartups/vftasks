/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

#include "vftasks.h"

#include <stdlib.h>     /* for malloc, free, and abort */
#include <stdio.h>      /* for printing to stderr */
#include <semaphore.h>  /* semaphores */

/* not all compilers recognize __inline__ */
#ifndef __GNUC__
#define __inline__
#endif

/* ***************************************************************************
 * Two-dimensional synchronization between tasks
 * ***************************************************************************/

/** 2D-synchronization manager
 */
struct vftasks_2d_sync_mgr_s
{
  int dim_x;    /* iteration-space size along x-dimension */
  int dim_y;    /* iteration-space size along y-dimension */
  int dist_x;   /* critical distance along x-dimension */
  int dist_y;   /* critical distance along y-dimension */
  sem_t *sems;  /* pointer to an array of dim_x semaphores */
};

/** abort
 */
void abort_on_fail(char *msg)
{
#ifdef VFTASKS_ABORT_ON_FAILURE
  fprintf(stderr, "Failure: %s\n", msg);
  abort();
#endif
}

/** create a 2D-synchronization manager
 */
vftasks_2d_sync_mgr_t *vftasks_create_2d_sync_mgr(int dim_x,
                                                  int dim_y,
                                                  int dist_x,
                                                  int dist_y)
{
  vftasks_2d_sync_mgr_t *mgr;  /* pointer to the manager */
  int x;                 /* index */

  if (abs(dist_x) >= dim_x || abs(dist_y) >= dim_y)
  {
    abort_on_fail("vftasks_create_2d_mgr: distance larger than dimension");
    return NULL;
  }

  /* allocate a manager */
  mgr = (vftasks_2d_sync_mgr_t *)malloc(sizeof(vftasks_2d_sync_mgr_t));
  if (mgr == NULL)
  {
    abort_on_fail("vftasks_create_2d_mgr: not enough memory");
    return NULL;
  }

  /* set the data for manager */
  mgr->dim_x = dim_x;
  mgr->dim_y = dim_y;
  mgr->dist_x = dist_x;
  mgr->dist_y = dist_y;

  /* allocate the semaphores held by the manager */
  mgr->sems = (sem_t *)malloc(dim_x * sizeof(sem_t));
  if (mgr->sems == NULL)
  {
    free(mgr);
    abort_on_fail("vftasks_create_2d_mgr: not enough memory");
    return NULL;
  }

  /* initialize the semaphores */
  for (x = 0; x < dim_x; ++x)
  {
    sem_init(&mgr->sems[x], 0, 0);
  }

  /* return the pointer to the manager */
  return mgr;
}

/** destroy a 2D-synchronization manager
 */
void vftasks_destroy_2d_sync_mgr(vftasks_2d_sync_mgr_t *mgr)
{
  int x;  /* index */

  /* destroy the semaphores held by the manager */
  for (x = 0; x < mgr->dim_x; ++x)
  {
    sem_destroy(&mgr->sems[x]);
  }

  /* deallocate the semaphores */
  free(mgr->sems);

  /* deallocate the manager */
  free(mgr);
}

/** signal end of inner iteration
 */
int vftasks_signal_2d(vftasks_2d_sync_mgr_t *mgr, int x, int y)
{
  /* check whether it is necessary to signal to another outer iteration */
  if (x >= -mgr->dist_x && x < mgr->dim_x - mgr->dist_x &&
      y >= -mgr->dist_y && y < mgr->dim_y - mgr->dist_y)
  {
    /* signal through the current outer iteration's semaphore; on failure, return 1 */
    if(sem_post(&mgr->sems[x]))
    {
      abort_on_fail("vftasks_signal_2d");
      return 1;
    }
  }

  /* return 0 to indicate success */
  return 0;
}

/** synchronize at start of inner iteration
 */
int vftasks_wait_2d(vftasks_2d_sync_mgr_t *mgr, int x, int y)
{
  /* check whether it is necessary to wait for another outer iteration */
  if (x >= mgr->dist_x && x < mgr->dim_x + mgr->dist_x &&
      y >= mgr->dist_y && y < mgr->dim_y + mgr->dist_y)
  {
    /* wait through the other iteration's semaphore; on failure, return 1 */
    if (sem_wait(&mgr->sems[x - mgr->dist_x]))
    {
      abort_on_fail("vftasks_wait_2d");
      return 1;
    }
  }

  /* return 0 to indicate success */
  return 0;
}
