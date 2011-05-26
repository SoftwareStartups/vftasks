/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

/* Example: usage of worker thread pools.
 * Computations on an array are partitioned into 4 tasks which are distributed
 * among a pool of worker threads.
 */

#include <vftasks.h>

#include <stdio.h>
#include <stdlib.h>

#define M 1024
#define N_PARTITIONS 4

volatile int a[M];
vftasks_pool_t *pool;

/* pack function arguments in a struct */
typedef struct
{
  int start;
  int length;
} task_t;


/* Simple task that performs work on part of the array.
 * The returned result is the size of the array chunk that has been worked on.
 * The original loop looked like this:
 * for (i = 0; i < M; i++)
 *   a[i] = i * i;
 */
void *task(void *raw_args)
{
  int i;
  task_t *args = (task_t *)raw_args;
  int *result = malloc(sizeof(int)); /* still needed when out of scope */

  for (i = args->start; i < args->start + args->length; i++)
    a[i] = i * i;

  *result = i - args->start;

  return result;
}

int threading()
{
  int k, acc = 0;
  int *results[N_PARTITIONS];

  /* be sure to put the arguments on the heap as soon as the function submitting the
     tasks returns before vftasks_get is called on the workers */
  task_t args[N_PARTITIONS];

  /* start the workers */
  for (k = 0; k < N_PARTITIONS-1; k++)
  {
    args[k].start = k * M / N_PARTITIONS;
    args[k].length = M / N_PARTITIONS;
    vftasks_submit(pool, task, &args[k], 0);
  }

  /* wait for the workers to finish */
  for (k = 0; k < N_PARTITIONS-1; k++)
    vftasks_get(pool, (void **)&results[k]);

  return acc;
}

int main()
{
  int result, cnt = 100;
  uint64_t time;

  /* only three workers needed, since one task is executed by the main thread */
  pool = vftasks_create_pool(N_PARTITIONS-1, 1);

  while (cnt--)
  {
    vftasks_timer_start(&time);
    result = threading();
    printf("time elapsed %lu\n", vftasks_timer_stop(&time));
  }

  vftasks_destroy_pool(pool);

  return 0;
}
