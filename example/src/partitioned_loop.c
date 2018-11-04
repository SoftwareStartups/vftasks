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
void task(void *raw_args)
{
  int i;
  task_t *args = (task_t *)raw_args;

  for (i = args->start; i < args->start + args->length; i++)
    a[i] = i * i;
}

int go()
{
  int k, rc = 0;

  /* be sure to put the arguments on the heap as soon as the function submitting the
     tasks returns before vftasks_get is called on the workers */
  task_t args[N_PARTITIONS];

  /* start the workers */
  for (k = 0; k < N_PARTITIONS-1; k++)
  {
    args[k].start = k * M / N_PARTITIONS;
    args[k].length = M / N_PARTITIONS;
    rc |= vftasks_submit(pool, task, &args[k], 0);
  }

  /* keep main thread busy by keeping part of the work in there */
  args[k].start = k * M / N_PARTITIONS;
  args[k].length = M / N_PARTITIONS;
  task(&args[k]);

  /* wait for the workers to finish */
  for (k = 0; k < N_PARTITIONS-1; k++)
    rc |= vftasks_get(pool);

  return rc;
}

int test(int result)
{
  int i, acc = 0;

  for (i = 0; i < M; i++)
    acc += a[i];

  return (acc == 357389824) && (result == 0);
}

int main()
{
  int result;

  /* only three workers needed, since one task is executed by the main thread */
  pool = vftasks_create_pool(N_PARTITIONS-1, 0);

  result = go();

  vftasks_destroy_pool(pool);

  if (test(result))
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
