// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#ifndef TASKSTEST_H
#define TASKSTEST_H

#include <cppunit/extensions/HelperMacros.h>

extern "C"
{
#include <vftasks.h>
}

#define ROWS 32
#define COLS 32
#define N_PARTITIONS 4

typedef struct
{
  int start;
  int stride;
} loop_args_t;

typedef struct
{
  int start;
  int stride;
  vftasks_pool_t *pool;
} outer_loop_args_t;

typedef struct
{
  int outer_loop_idx;
  int start;
  int stride;
} inner_loop_args_t;

class TasksTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TasksTest);

  CPPUNIT_TEST(testCreateEmptyPool);
  CPPUNIT_TEST(testCreateInvalidPool);
  CPPUNIT_TEST(testCreatePool1);
  CPPUNIT_TEST(testCreatePool4);
  CPPUNIT_TEST(testDestroyPool);

  CPPUNIT_TEST(testSubmitEmptyTask);
  CPPUNIT_TEST(testSubmit);
  CPPUNIT_TEST(testSubmitInvalidNumWorkers);
  CPPUNIT_TEST(testSubmitGet);
  CPPUNIT_TEST(testGetNoWorkers);
  CPPUNIT_TEST(testSubmitLoop);
  CPPUNIT_TEST(testSubmitGetLoop);
  CPPUNIT_TEST(testTooManyGets);

  CPPUNIT_TEST(testSubmitNestedLoop);
  CPPUNIT_TEST(testSubmitNestedLoopInvalidSubWorkers);
  CPPUNIT_TEST(testSubmitGetNestedLoop);
  CPPUNIT_TEST(testSubmitGetNestedLoopInvalidSubWorkers);

  CPPUNIT_TEST_SUITE_END();  // TasksTest

public:
  void testCreateEmptyPool();
  void testCreateInvalidPool();
  void testCreatePool1();
  void testCreatePool4();
  void testDestroyPool();

  void testSubmitEmptyTask();
  void testSubmit();
  void testSubmitInvalidNumWorkers();
  void testSubmitGet();
  void testGetNoWorkers();
  void testSubmitLoop();
  void testSubmitGetLoop();
  void testTooManyGets();

  void testSubmitNestedLoop();
  void testSubmitNestedLoopInvalidSubWorkers();
  void testSubmitGetNestedLoop();
  void testSubmitGetNestedLoopInvalidSubWorkers();

  void tearDown();

private:
  void submitLoop();
  int submitNestedLoop(int numWorkers);
  int submitGetNestedLoop(int numWorkers, int expectedResult);

  vftasks_pool_t *pool;  // pointer to a worker-thread pool
};

#endif  // TASKSTEST_H
