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

#define ROWS 32
#define COLS 32
#define N_PARTITIONS 4

volatile static int array[ROWS];
volatile static int matrix[ROWS][COLS];

// a task that computes the square of an integer
// can be used as a task without subsidiary workers
static void *square(void *raw_args)
{
  square_args_t *arg;     // a pointer to the argument
  int result;

  // cast the argument pointer to a pointer of the proper type
  arg = (square_args_t *)raw_args;

  // compute the square of the argument and store in the memory allocated for the result
  result = arg->val * arg->val;

  if (arg->result_needed)
  {
    int *result_ptr = (int *)malloc(sizeof(int));
    *result_ptr = result;
    return result_ptr;
  }

  // return a pointer to the result
  return NULL;
}

TasksTest::TasksTest()
{
  this->pool = NULL;
  this->square_args = NULL;
  this->loop_args = NULL;
  this->inner_loop_args = NULL;
  this->outer_loop_args = NULL;
}

void TasksTest::tearDown()
{
  if (this->pool != NULL)
    vftasks_destroy_pool(this->pool);
  if (this->square_args !=NULL)
    free(this->square_args);
  if (this->loop_args !=NULL)
    free(this->loop_args);
  if (this->inner_loop_args !=NULL)
    free(this->inner_loop_args);
  if (this->outer_loop_args !=NULL)
    free(this->outer_loop_args);
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
  this->square_args = (square_args_t *)malloc(sizeof(square_args_t));
  this->square_args->val = 3;
  this->square_args->result_needed = false;

  this->pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &this->square_args, 0) == 0);
}

void TasksTest::testSubmitInvalidNumWorkers()
{
  this->square_args = (square_args_t *)malloc(sizeof(square_args_t));
  this->square_args->val = 3;
  this->square_args->result_needed = false;
  this->pool = vftasks_create_pool(1);

  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &this->square_args, -1) != 0);
  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, &this->square_args, 1) != 0);
}

void TasksTest::testSubmitGet()
{
  int *result_ptr;
  this->square_args = (square_args_t *)malloc(sizeof(square_args_t));
  this->square_args->val = 3;
  this->square_args->result_needed = true;
  this->pool = vftasks_create_pool(1);

  CPPUNIT_ASSERT(vftasks_submit(this->pool, square, this->square_args, 0) == 0);
  CPPUNIT_ASSERT(vftasks_get(this->pool, (void **)&result_ptr) == 0);
  CPPUNIT_ASSERT(*result_ptr == 9);
  free(result_ptr);
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
  loop_args_t *args = (loop_args_t *)raw_args;

  for (i = args->start; i < ROWS; i += args->stride)
    array[i] = i;

  return NULL;
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
      result |= vftasks_get(args->pool, NULL);
  }

  if (args->result_needed)
  {
    int *result_ptr = (int *)malloc(sizeof(int));
    *result_ptr = result;
    return result_ptr;
  }

  return NULL;
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
    this->outer_loop_args[i].result_needed = false;
    results[i] = vftasks_submit(this->pool, outer_loop,
                                &this->outer_loop_args[i], numWorkers);
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
  this->outer_loop_args = (outer_loop_args_t *)calloc(N_PARTITIONS,
                                                      sizeof(outer_loop_args_t));

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    this->outer_loop_args[i].start = i;
    this->outer_loop_args[i].stride = N_PARTITIONS;
    this->outer_loop_args[i].pool = this->pool;
    this->outer_loop_args[i].result_needed = true;
    vftasks_submit(this->pool, outer_loop, &this->outer_loop_args[i], numWorkers);
  }

  for (i = 0; i < N_PARTITIONS-1; i++)
  {
    result_ptr = NULL;
    results[i] = vftasks_get(this->pool, (void **)&result_ptr);
    if (result_ptr != NULL)
    {
      result |= *result_ptr;
      free(result_ptr);
    }
  }

  this->outer_loop_args[i].start = i;
  this->outer_loop_args[i].stride = N_PARTITIONS;
  this->outer_loop_args[i].pool = this->pool;
  this->outer_loop_args[i].result_needed = true;
  result_ptr = NULL;
  result_ptr = (int *)outer_loop(&this->outer_loop_args[i]);
  if (result_ptr != NULL)
  {
    result |= *result_ptr;
    free(result_ptr);
  }

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
