/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

/* Example: usage of worker thread pools with 2D synchronization
 * Computations on a 2-dimensional array are partitioned into 4 tasks
 * which are distributed among a pool of worker threads.
 * An inter-task dependency exists (note the branch in the nested loop body),
 * which is synchronized using the 2D synchronization API.
 * The 2d.c file contains the original (unpartitioned) code.
 */

#include <stdio.h>

#include <vftasks.h>

#define M 1024
#define N 1024
#define N_PARTITIONS 4

volatile int a[M][N];
vftasks_pool_t *pool;
vftasks_2d_sync_mgr_t *sync_mgr;

/* pack original function arguments in a struct */
typedef struct
{
  int start;
  int stride;
} task_t;

void *task(void *raw_args)
{
  task_t *args = (task_t *)raw_args;

  int i, j;

  for (i = args->start; i < M; i += args->stride)
  {
    for (j = 0; j < N; j++)
    {
      /* synchronize with other partition */
      vftasks_wait_2d(sync_mgr, i, j);

      if (i > 0 && j + 1 < N) /* this causes an inter-task dependency */
      {
        a[i][j] = i * j + a[i - 1][j + 1];
      }
      else
      {
        a[i][j] = i * j;
      }

      /* signal other waiting partitions */
      vftasks_signal_2d(sync_mgr, i, j);
    }
  }

  return NULL;
}

void go()
{
  int k;

  /* be sure to put the arguments on the heap as soon as the function submitting the
     tasks returns before vftasks_get is called on the workers */
  task_t args[N_PARTITIONS];

  sync_mgr = vftasks_create_2d_sync_mgr(M, N, 1, -1);

  /* start the workers */
  for (k = 0; k < N_PARTITIONS-1; k++)
  {
    args[k].start = k;
    args[k].stride = N_PARTITIONS;
    vftasks_submit(pool, task, &args[k], 0);
  }

  /* keep main thread busy by keeping part of the work in there */
  args[k].start = k;
  args[k].stride = N_PARTITIONS;
  task(&args[k]);

  /* wait for the workers to finish */
  for (k = 0; k < N_PARTITIONS-1; k++)
  {
    vftasks_get(pool, NULL);
  }

  vftasks_destroy_2d_sync_mgr(sync_mgr);
}

int test()
{
  int i, j;
  int acc = 0;

  for (i = 0; i < M; i++)
  {
    for (j = 0; j < N; j++)
    {
      acc += a[i][j];
    }
  }

  return (acc == 438488320);
}

int main()
{
  /* one task is executed by the main thread */
  pool = vftasks_create_pool(N_PARTITIONS-1);

  go();

  vftasks_destroy_pool(pool);

  if (test())
  {
    printf("PASSED\n");
    return 0;
  }
  else
  {
    printf("FAILED\n");
    return 1;
  }
}
