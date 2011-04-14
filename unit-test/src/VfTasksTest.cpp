// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#include "VfTasksTest.h"

#include <cstdlib>  // for malloc, free

#define X 32
#define y 32
#define N_PARTITIONS 4

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

void VfTasksTest::tearDown()
{
  if (this->pool != NULL)
    vftasks_destroy_pool(pool);

  if (this->args != NULL)
    free(args);
}

void VfTasksTest::testCreateEmptyPool()
{
  vftasks_pool_t *pool = vftasks_create_pool(0);
  CPPUNIT_ASSERT(pool == NULL);
}

void VfTasksTest::testCreateInvalidPool()
{
  vftasks_pool_t *pool = vftasks_create_pool(-1);
  CPPUNIT_ASSERT(pool == NULL);
}

void VfTasksTest::testCreatePool1()
{
  vftasks_pool_t *pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(pool != NULL);
  vftasks_destroy_pool(pool);
}

void VfTasksTest::testCreatePool4()
{
  vftasks_pool_t *pool = vftasks_create_pool(4);
  CPPUNIT_ASSERT(pool != NULL);
}

void VfTasksTest::testDestroyPool()
{
  vftasks_pool_t *pool = vftasks_create_pool(4);
  vftasks_destroy_pool(pool);
}

void VfTasksTest::testSubmitEmptyTask()
{
  this->pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(vftasks_submit(pool, NULL, NULL, 0) != 0);
}

void VfTasksTest::testSubmit()
{
  int arg = 3;

  this->pool = vftasks_create_pool(1);
  CPPUNIT_ASSERT(vftasks_submit(pool, square, &arg, 0) == 0);
}

void VfTasksTest::testSubmitInvalidNumWorkers()
{
  int arg = 3;

  this->pool = vftasks_create_pool(1);

  CPPUNIT_ASSERT(vftasks_submit(pool, square, &arg, -1) != 0);
  CPPUNIT_ASSERT(vftasks_submit(pool, square, &arg, 1) != 0);
}

void VfTasksTest::testSubmitGet()
{
  int arg = 3;
  int *result_ptr;

  this->pool = vftasks_create_pool(1);

  CPPUNIT_ASSERT(vftasks_submit(pool, square, &arg, 0) == 0);
  CPPUNIT_ASSERT(vftasks_get(pool, (void **)&result_ptr) == 0);
  CPPUNIT_ASSERT(*result_ptr == 9);
}

static void *loop(void *raw_args)
{
  args_t *args = (args_t *)raw_args;

  int i;
  int *acc = (int *)malloc(sizeof(int));
  *acc = 0;

  for (i = args->start; i < X; i += args->stride)
    *acc += i;

  return acc;
}

void VfTasksTest::submit_loop()
{
  int k;

  this->args = (args_t *)calloc(N_PARTITIONS, sizeof(args_t));

  for (k = 0; k < N_PARTITIONS; k++)
  {
    args[k].start = k;
    args[k].stride = N_PARTITIONS;
    CPPUNIT_ASSERT(vftasks_submit(this->pool, loop, &this->args[k], 0) == 0);
  }
}

int VfTasksTest::get_loop()
{
  int k;
  int acc = 0;
  int *result;

  for (k = 0; k < N_PARTITIONS; k++)
  {

    CPPUNIT_ASSERT(vftasks_get(pool, (void **)&result) == 0);
    acc+= *result;
  }

  return acc;
}

void VfTasksTest::testSubmitLoop()
{
  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submit_loop();
}

void VfTasksTest::testSubmitGetLoop()
{
  this->pool = vftasks_create_pool(N_PARTITIONS);
  this->submit_loop();
  CPPUNIT_ASSERT(get_loop() == 496);
}

void *nested_loop(void *raw_args)
{
  args_t *args = (args_t *)raw_args;

  int i;
  int *acc = (int *)malloc(sizeof(int));
  *acc = 0;

  for (i = args->start; i < X; i += args->stride)
    *acc += i;

  return acc;
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(VfTasksTest);
