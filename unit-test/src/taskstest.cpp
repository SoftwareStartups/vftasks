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

static void *loop(void *raw_args)
{
  int i;
  int *acc = (int *)malloc(sizeof(int));
  loop_args_t *args = (loop_args_t *)raw_args;

  *acc = 0;

  for (i = args->start; i < ROWS; i += args->stride)
    args->array[i] = i;

  return NULL;
}

void TasksTest::submit_loop()
{
  int k;

  // this is not freed intentionally, since the workers might still be running
  // when it goes out of scope
  loop_args_t *args = (loop_args_t *)calloc(N_PARTITIONS, sizeof(loop_args_t));

  for (k = 0; k < N_PARTITIONS; k++)
  {
    args[k].start = k;
    args[k].stride = N_PARTITIONS;
    args[k].array = this->array;
    CPPUNIT_ASSERT(vftasks_submit(this->pool, loop, &args[k], 0) == 0);
  }
}

void TasksTest::testSubmitLoop()
{
  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submit_loop();
}

void TasksTest::testSubmitGetLoop()
{
  int k;
  int *result;

  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submit_loop();

  for (k = 0; k < N_PARTITIONS; k++)
    CPPUNIT_ASSERT(vftasks_get(this->pool, (void **)&result) == 0);
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(TasksTest);
