#include "integrationtest.h"

#define ROWS 32
#define COLS 32

void IntegrationTest::setUp()
{
  this->pool = vftasks_create_pool(N_PARTITIONS*N_PARTITIONS, 0);
  this->sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, 1, -1);

  for (int i = 0; i < N_PARTITIONS; i++)
  {
    this->outer_args[i].pool = this->pool;
    this->outer_args[i].sync_mgr = this->sync_mgr;
  }
}

void IntegrationTest::tearDown()
{
  vftasks_destroy_pool(this->pool);
  vftasks_destroy_2d_sync_mgr(sync_mgr);
}

// A task-like function that contains the inner loop part of the outer loop
// described below.
static void inner_loop(void *raw_args)
{
  int i, j;
  inner_loop_args_t *args = (inner_loop_args_t *)raw_args;

  i = args->outer_loop_idx;
  for (j = args->start; j < COLS; j += args->stride)
  {
    // these waits cause the actual deadlocks as a result of the old fifo scheduling
    vftasks_wait_2d(args->sync_mgr, i, j);
    vftasks_signal_2d(args->sync_mgr, i, j);
  }
}

// A task-like function that contains a nested loop.
// The inner loop is called by submitting subsidiary tasks.
static void outer_loop(void *raw_args)
{
  int i, j, result = 0;
  inner_loop_args_t inner_args[N_PARTITIONS];
  outer_loop_args_t *args = (outer_loop_args_t *) raw_args;

  for (i = args->start; i < ROWS; i += args->stride)
  {
    for (j = 0; j < N_PARTITIONS-1; j++)
    {
      inner_args[j].outer_loop_idx = i;
      inner_args[j].start = j;
      inner_args[j].stride = N_PARTITIONS;
      inner_args[j].sync_mgr = args->sync_mgr;
      result |= vftasks_submit(args->pool, inner_loop, &inner_args[j], 0);
    }

    inner_args[j].outer_loop_idx = i;
    inner_args[j].start = j;
    inner_args[j].stride = N_PARTITIONS;
    inner_args[j].sync_mgr = args->sync_mgr;
    inner_loop(&inner_args[j]);

    for (j = 0; j < N_PARTITIONS-1; j++)
      result |= vftasks_get(args->pool);
  }

  CPPUNIT_ASSERT(result == 0);
}

void IntegrationTest::testDeadlock()
{
  int i, result = 0;

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    this->outer_args[i].start = i;
    this->outer_args[i].stride = N_PARTITIONS;
    this->outer_args[i].pool = this->pool;
    result |= vftasks_submit(this->pool,
                             outer_loop,
                             &this->outer_args[i],
                             N_PARTITIONS-1);
  }

  this->outer_args[i].start = i;
  this->outer_args[i].stride = N_PARTITIONS;
  this->outer_args[i].pool = this->pool;
  outer_loop(&this->outer_args[i]);

  for (i = 0; i < N_PARTITIONS-1; i++)
    result |= vftasks_get(this->pool);
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(IntegrationTest);
