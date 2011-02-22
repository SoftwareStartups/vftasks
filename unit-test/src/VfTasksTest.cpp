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

// a task that computes the square of an integer
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

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(VfTasksTest);

void VfTasksTest::testExecution()
{
  vftasks_task_t *task;  // pointer to a task
  int *arg;              // pointer to the argument of the task
  int *result;           // pointer to the constructed result

  // select a task
  task = square;

  // allocate memory for the argument and the result
  arg = (int *)malloc(sizeof(int));
  CPPUNIT_ASSERT(arg != NULL);
  result = (int *)malloc(sizeof(int));
  CPPUNIT_ASSERT(result != NULL);

  // store the argument
  *arg = 42;

  // create a thread pool
  pool = vftasks_create_pool(7);
  CPPUNIT_ASSERT(pool != NULL);

  // submit the task
  CPPUNIT_ASSERT(vftasks_submit(this->pool, task, arg, 0) == 0);

  // retrieve the result and verify that it's correct
  CPPUNIT_ASSERT(vftasks_get(pool, (void **)&result) == 0);
  CPPUNIT_ASSERT(*result == 42 * 42);

  // destroy the thread pool
  vftasks_destroy_pool(pool);

  // deallocate the memory for the argument and the result
  free(arg);
  free(result);
}
