// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#include "taskstest.h"

#include <cstdlib>  // for malloc, free

#define ROWS 8
#define COLS 8
#define N_PARTITIONS 2

volatile static int array[ROWS];
volatile static int matrix[ROWS][COLS];

// a task that computes the square of an integer
// can be used as a task without subsidiary workers
static void square(void *raw_args)
{
  square_args_t *arg = (square_args_t *)raw_args;

  // compute the square of the argument and store in the memory allocated for the result
  arg->result = arg->val * arg->val;
}

void TasksTest::setUp()
{
  this->pool = NULL;
  this->square_args = NULL;
  this->loop_args = NULL;
  this->inner_loop_args = NULL;
  this->outer_loop_args = NULL;
  this->busy_wait = 0;
}

void TasksTest::tearDown()
{
  if (this->pool != NULL)
    vftasks_destroy_pool(this->pool);
  if (this->square_args != NULL)
    free(this->square_args);
  if (this->loop_args != NULL)
    free(this->loop_args);
  if (this->inner_loop_args != NULL)
    free(this->inner_loop_args);
  if (this->outer_loop_args != NULL)
    free(this->outer_loop_args);
}

vftasks_pool_t *TasksTest::createPool(int numWorkers)
{
  return vftasks_create_pool(numWorkers, this->busy_wait);
}

void TasksTest::testCreateEmptyPool()
{
  this->pool = createPool(0);
  CPPUNIT_ASSERT(this->pool == NULL);
}

void TasksTest::testCreateInvalidPool()
{
  this->pool = createPool(-1);
  CPPUNIT_ASSERT(this->pool == NULL);
}

void TasksTest::testCreatePool1()
{
  vftasks_pool_t *pool = createPool(1);
  CPPUNIT_ASSERT(pool != NULL);
  vftasks_destroy_pool(pool);
}

void TasksTest::testCreatePool4()
{
  this->pool = createPool(4);
  CPPUNIT_ASSERT(pool != NULL);
}

void TasksTest::testDestroyPool()
{
  vftasks_pool_t *pool = createPool(4);
  vftasks_destroy_pool(pool);
}

void TasksTest::testSubmitEmptyTask()
{
  this->pool = createPool(1);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, NULL, NULL, 0) != 0);
}

void TasksTest::testSubmit()
{
  this->square_args = (square_args_t *)malloc(sizeof(square_args_t));
  this->square_args->val = 3;

  this->pool = createPool(1);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, this->square_args, 0) == 0);
}

void TasksTest::testSubmitInvalidNumWorkers()
{
  this->square_args = (square_args_t *)malloc(sizeof(square_args_t));
  this->square_args->val = 3;
  this->pool = createPool(1);

  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &this->square_args, -1) != 0);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &this->square_args, 1) != 0);
}

void TasksTest::testSubmitGet()
{
  this->square_args = (square_args_t *)malloc(sizeof(square_args_t));
  this->square_args->val = 3;
  this->pool = createPool(1);

  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, this->square_args, 0) == 0);
  CPPUNIT_ASSERT(vftasks_get(this->pool) == 0);
  CPPUNIT_ASSERT(this->square_args->result == 9);
}

void TasksTest::testGetNoWorkers()
{
  this->pool = createPool(1);
  CPPUNIT_ASSERT(vftasks_get(pool) != 0);
}

// A simple loop task.
static void loop(void *raw_args)
{
  int i;
  loop_args_t *args = (loop_args_t *)raw_args;

  for (i = args->start; i < ROWS; i += args->stride)
    array[i] = i;
}

void TasksTest::submitLoop()
{
  int i;

  // this is not freed intentionally, since the workers might still be running
  // when it goes out of scope
  this->loop_args = (loop_args_t *)calloc(N_PARTITIONS, sizeof(loop_args_t));

  for (i = 0; i < N_PARTITIONS; i++)
  {
    this->loop_args[i].start = i;
    this->loop_args[i].stride = N_PARTITIONS;
    CPPUNIT_ASSERT(vftasks_submit(this->pool, loop, &this->loop_args[i], 0) == 0);
  }
}

void TasksTest::testSubmitLoop()
{
  this->pool = createPool(N_PARTITIONS);
  this->submitLoop();
}

void TasksTest::testSubmitGetLoop()
{
  int k;

  this->pool = createPool(N_PARTITIONS);
  this->submitLoop();

  for (k = 0; k < N_PARTITIONS; k++)
    CPPUNIT_ASSERT(vftasks_get(this->pool) == 0);
}

void TasksTest::testTooManyGets()
{
  int k;

  this->pool = createPool(N_PARTITIONS);
  this->submitLoop();

  for (k = 0; k < N_PARTITIONS; k++)
    CPPUNIT_ASSERT(vftasks_get(this->pool) == 0);

  CPPUNIT_ASSERT(vftasks_get(this->pool) != 0);
}

// A task-like function that contains the inner loop part of the outer loop
// described below.
static void inner_loop(void *raw_args)
{
  int i, j;
  inner_loop_args_t *args = (inner_loop_args_t *)raw_args;

  i = args->outer_loop_idx;
  for (j = args->start; j < COLS; j += args->stride)
    matrix[i][j] = i*j;
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
      vftasks_submit(args->pool, inner_loop, &inner_args[j], 0);
    }

    inner_args[j].outer_loop_idx = i;
    inner_args[j].start = j;
    inner_args[j].stride = N_PARTITIONS;
    inner_loop(&inner_args[j]);

    for (j = 0; j < N_PARTITIONS-1; j++)
      result |= vftasks_get(args->pool);
  }

  args->result = result;
}

// Helper function to submit nested loop tasks with a variable
// number of subsidiary workers. The vftasks_submit returncodes are OR-ed
// so the calling test functions can assert them.
int TasksTest::submitNestedLoop(int numWorkers)
{
  int i, result = 0;
  int results[N_PARTITIONS-1];
  this->outer_loop_args = (outer_loop_args_t *)calloc(N_PARTITIONS,
                                                      sizeof(outer_loop_args_t));

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    this->outer_loop_args[i].start = i;
    this->outer_loop_args[i].stride = N_PARTITIONS;
    this->outer_loop_args[i].pool = this->pool;
    results[i] = vftasks_submit(this->pool, outer_loop,
                                &this->outer_loop_args[i], numWorkers);
  }

  for (i = 0; i < N_PARTITIONS-1; i++)
    result |= results[i];

  return result;
}

void TasksTest::testSubmitNestedLoop()
{
  this->pool = createPool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitNestedLoop(N_PARTITIONS) == 0);
}

void TasksTest::testSubmitNestedLoopInvalidSubWorkers()
{
  this->pool = createPool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitNestedLoop(N_PARTITIONS+1) != 0);
}

// Helper function to submit and get nested loop tasks with a variable
// number of subsidiary workers.
// The vftasks_get returncodes are OR-ed so the calling test functions can
// assert them.
// An internal assert is done on the OR-ed results from the vftasks_get calls.
// This assert is done with the expectedResult function parameter.
int TasksTest::submitGetNestedLoop(int numWorkers, int expectedResult)
{
  int i, result = 0;
  int results[N_PARTITIONS-1];
  this->outer_loop_args = (outer_loop_args_t *)calloc(N_PARTITIONS,
                                                      sizeof(outer_loop_args_t));

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    this->outer_loop_args[i].start = i;
    this->outer_loop_args[i].stride = N_PARTITIONS;
    this->outer_loop_args[i].pool = this->pool;
    result |= vftasks_submit(this->pool,
                             outer_loop,
                             &this->outer_loop_args[i],
                             numWorkers);
  }

  if (result == 0)
  {
    for (i = 0; i < N_PARTITIONS-1; i++)
    {
      results[i] = vftasks_get(this->pool);
      result |= outer_loop_args[i].result;
    }

    this->outer_loop_args[i].start = i;
    this->outer_loop_args[i].stride = N_PARTITIONS;
    this->outer_loop_args[i].pool = this->pool;
    outer_loop(&this->outer_loop_args[i]);
    result |= outer_loop_args[i].result;

    CPPUNIT_ASSERT(result == expectedResult);

    for (i = 0; i < N_PARTITIONS-1; i++)
      result |= results[i];
  }

  return result;
}

void TasksTest::testSubmitGetNestedLoop()
{
  this->pool = createPool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitGetNestedLoop(N_PARTITIONS, 0) == 0);
}

void TasksTest::testSubmitGetNestedLoopInvalidSubWorkers()
{
  this->pool = createPool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitGetNestedLoop(N_PARTITIONS+1, 0) != 0);
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(TasksTest);
