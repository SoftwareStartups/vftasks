/* Example: usage of worker thread pools.
 * Computations on an array are partitioned into 4 tasks which are distributed
 * among a pool of worker threads.
 * The partitioning will not result in any speedup because the amount of work in
 * the tasks is too small. This is intentional since the aim is to showcase the API.
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
  int result;
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

  args->result = i - args->start;
}

int threading()
{
  int k, acc = 0;

  /* put the arguments on the heap so the worker threads can access them */
  task_t *args = calloc(N_PARTITIONS, sizeof(task_t));

  /* start the workers */
  for (k = 0; k < N_PARTITIONS-1; k++)
  {
    args[k].start = k * M / N_PARTITIONS;
    args[k].length = M / N_PARTITIONS;
    vftasks_submit(pool, task, &args[k], 0);
  }

  task(&args[k]);

  /* wait for the workers to finish */
  for (k = 0; k < N_PARTITIONS-1; k++)
    vftasks_get(pool);

  for (k = 0; k < N_PARTITIONS; k++)
    acc += args[k].result;

  free(args);

  return acc;
}

int main()
{
  int result = 0, cnt = 100;
  uint64_t time;

  /* only three workers needed, since one task is executed by the main thread */
  pool = vftasks_create_pool(N_PARTITIONS-1, 1);

  while (cnt--)
  {
    vftasks_timer_start(&time);
    result += threading();
    printf("time elapsed %lu\n", vftasks_timer_stop(&time));
  }

  vftasks_destroy_pool(pool);

  return result == 76800 ? 0 : result;
}
