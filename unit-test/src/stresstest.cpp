#include "stresstest.h"

#include <stdlib.h>

#define THREAD_MULTIPLY_FACTOR 10
#define LEVELS 4
#define SEED 13
#define DELAY 1000000

static volatile int dummy;
static vftasks_pool_t *pool;

void StressTest::setUp()
{
  pool = vftasks_create_pool(N_PARTITIONS*N_PARTITIONS, 0);
  srand(SEED);
}

void StressTest::tearDown()
{
  vftasks_destroy_pool(pool);
}

static void delay(int max)
{
  int i;

  for (i = 0; i < max; i++)
    dummy *= i;
}

/* compute extra subsidary threads required by a task */
static int extra_threads(int levels)
{
  int threads, lvl_threads, lvl;

  for (lvl = 1, threads = 0, lvl_threads = THREAD_MULTIPLY_FACTOR;
       lvl <= levels;
       lvl++)
  {
    threads += lvl_threads;
    lvl_threads *= THREAD_MULTIPLY_FACTOR;
  }

  return threads;
}

/* submit tasks to worker threads */
static void nested_thread(void *thr_level_p)
{
  int k, extras;
  int thr_level = *(int *)thr_level_p;

  if (!thr_level)
    return;

  thr_level--;
  extras = extra_threads(thr_level);

  /* start the workers */
  for (k = 0; k < THREAD_MULTIPLY_FACTOR; k++)
  {
    vftasks_submit(pool, nested_thread, &thr_level, extras);
  }

  /* delay if RANDOM() returns an odd number */
  delay(rand() % 2 ? DELAY : 1);

  /* wait for the workers to finish */
  for (k = 0; k < THREAD_MULTIPLY_FACTOR; k++)
    vftasks_get(pool);
}

void StressTest::testStress()
{
  int cnt;

  for (cnt = 1; cnt > 0; cnt--)
  {
    int levels = LEVELS;
    nested_thread(&levels);
  }
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(StressTest);
