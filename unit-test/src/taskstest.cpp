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

volatile static int array[ROWS];
volatile static int matrix[ROWS][COLS];

// a task that computes the square of an integer
// can be used as a task without subsidiary workers
static void *square(void *raw_args)
{
  int *arg;     // a pointer to the argument
  int *result;  // a pointer to the result

  // cast the argument pointer to a pointer of the proper type
  arg = (int *)raw_args;

  // allocate memory for the result
  result = (int *)malloc(sizeof(int));

  // compute the square of the argument and store in the memory allocated for the result
  *result = *arg * *arg;

  // return a pointer to the result
  return result;
}

void TasksTest::tearDown()
{
  if (this->pool != NULL)
    vftasks_destroy_pool(this->pool);
}

void TasksTest::testCreateEmptyPool()
{
  this->pool = vftasks_create_pool(0);
  CPPUNIT_ASSERT(this->pool == NULL);
}

void TasksTest::testCreateInvalidPool()
{
  this->pool = vftasks_create_pool(-1);
  CPPUNIT_ASSERT(this->pool == NULL);
}

void TasksTest::testCreatePool1()
{
  vftasks_pool_t *pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(pool != NULL);
  vftasks_destroy_pool(pool);
}

void TasksTest::testCreatePool4()
{
  this->pool = vftasks_create_pool(4);
  CPPUNIT_ASSERT(pool != NULL);
}

void TasksTest::testDestroyPool()
{
  vftasks_pool_t *pool = vftasks_create_pool(4);
  vftasks_destroy_pool(pool);
}

void TasksTest::testSubmitEmptyTask()
{
  this->pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, NULL, NULL, 0) != 0);
}

void TasksTest::testSubmit()
{
  // this is not freed intentionally, since the workers might still be running
  // when it goes out of scope
  int *arg = (int *)malloc(sizeof(int));
  *arg = 3;

  this->pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &arg, 0) == 0);
}

void TasksTest::testSubmitInvalidNumWorkers()
{
  int arg = 3;

  this->pool = vftasks_create_pool(1);

  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &arg, -1) != 0);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &arg, 1) != 0);
}

void TasksTest::testSubmitGet()
{
  int arg = 3;
  int *result_ptr;

  this->pool = vftasks_create_pool(1);

  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &arg, 0) == 0);
  CPPUNIT_ASSERT(vftasks_get(this->pool, (void **)&result_ptr) == 0);
  CPPUNIT_ASSERT(*result_ptr == 9);
}

void TasksTest::testGetNoWorkers()
{
  this->pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(vftasks_get(pool, NULL) != 0);
}

// A simple loop task.
static void *loop(void *raw_args)
{
  int i;
  int *acc = (int *)malloc(sizeof(int));
  loop_args_t *args = (loop_args_t *)raw_args;

  *acc = 0;

  for (i = args->start; i < ROWS; i += args->stride)
    array[i] = i;

  return NULL;
}

void TasksTest::submitLoop()
{
  int i;

  // this is not freed intentionally, since the workers might still be running
  // when it goes out of scope
  loop_args_t *args = (loop_args_t *)calloc(N_PARTITIONS, sizeof(loop_args_t));

  for (i = 0; i < N_PARTITIONS; i++)
  {
    args[i].start = i;
    args[i].stride = N_PARTITIONS;
    CPPUNIT_ASSERT(vftasks_submit(this->pool, loop, &args[i], 0) == 0);
  }
}

void TasksTest::testSubmitLoop()
{
  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submitLoop();
}

void TasksTest::testSubmitGetLoop()
{
  int k;
  int *result;

  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submitLoop();

  for (k = 0; k < N_PARTITIONS; k++)
    CPPUNIT_ASSERT(vftasks_get(this->pool, (void **)&result) == 0);
}

void TasksTest::testTooManyGets()
{
  int k;
  int *result;

  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submitLoop();

  for (k = 0; k < N_PARTITIONS; k++)
    CPPUNIT_ASSERT(vftasks_get(this->pool, (void **)&result) == 0);

  CPPUNIT_ASSERT(vftasks_get(this->pool, (void **)&result) != 0);
}

// A task-like function that contains the inner loop part of the outer loop
// described below.
static void *inner_loop(void *raw_args)
{
  int i, j;
  inner_loop_args_t *args = (inner_loop_args_t *)raw_args;

  i = args->outer_loop_idx;
  for (j = args->start; j < COLS; j += args->stride)
    matrix[i][j] = i*j;

  return NULL;
}

// A task-like function that contains a nested loop.
// The inner loop is called by submitting subsidiary tasks.
static void *outer_loop(void *raw_args)
{
  int i, j;
  inner_loop_args_t inner_args[N_PARTITIONS];
  outer_loop_args_t *args = (outer_loop_args_t *) raw_args;
  int *result = (int *)malloc(sizeof(int));
  *result = 0;

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
      *result |= vftasks_get(args->pool, NULL);
  }

  return result;
}

// Helper function to submit nested loop tasks with a variable
// number of subsidiary workers. The vftasks_submit returncodes are OR-ed
// so the calling test functions can assert them.
int TasksTest::submitNestedLoop(int numWorkers)
{
  int i, result = 0;
  int results[N_PARTITIONS-1];
  outer_loop_args_t *args =
    (outer_loop_args_t *)calloc(N_PARTITIONS, sizeof(outer_loop_args_t));

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    args[i].start = i;
    args[i].stride = N_PARTITIONS;
    args[i].pool = this->pool;
    results[i] = vftasks_submit(this->pool, outer_loop, &args[i], numWorkers);
  }

  for (i = 0; i < N_PARTITIONS-1; i++)
    result |= results[i];

  return result;
}

void TasksTest::testSubmitNestedLoop()
{
  this->pool = vftasks_create_pool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitNestedLoop(N_PARTITIONS) == 0);
}

void TasksTest::testSubmitNestedLoopInvalidSubWorkers()
{
  this->pool = vftasks_create_pool(N_PARTITIONS * N_PARTITIONS - 1);
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
  int *result_ptr;
  outer_loop_args_t *args =
    (outer_loop_args_t *)calloc(N_PARTITIONS, sizeof(outer_loop_args_t));

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    args[i].start = i;
    args[i].stride = N_PARTITIONS;
    args[i].pool = this->pool;
    vftasks_submit(this->pool, outer_loop, &args[i], numWorkers);
  }

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    results[i] = vftasks_get(this->pool, (void **)&result_ptr);
    result |= *result_ptr;
  }

  args[i].start = i;
  args[i].stride = N_PARTITIONS;
  args[i].pool = this->pool;
  result |= *(int *)outer_loop(&args[i]);

  CPPUNIT_ASSERT(result == expectedResult);

  for (i = 0; i < N_PARTITIONS-1; i++)
    result |= results[i];

  return result;
}

void TasksTest::testSubmitGetNestedLoop()
{
  this->pool = vftasks_create_pool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitGetNestedLoop(N_PARTITIONS, 0) == 0);
}

void TasksTest::testSubmitGetNestedLoopInvalidSubWorkers()
{
  this->pool = vftasks_create_pool(N_PARTITIONS * N_PARTITIONS - 1);
  CPPUNIT_ASSERT(submitGetNestedLoop(N_PARTITIONS+1, 0) != 0);
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(TasksTest);
