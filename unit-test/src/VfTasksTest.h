// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#ifndef VFTASKSTEST_H
#define VFTASKSTEST_H

#include <cppunit/extensions/HelperMacros.h>

extern "C"
{
#include <vftasks.h>
}

typedef struct
{
  int start;
  int stride;
} args_t;

class VfTasksTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(VfTasksTest);

  CPPUNIT_TEST(testCreateEmptyPool);
  CPPUNIT_TEST(testCreateInvalidPool);
  CPPUNIT_TEST(testCreatePool1);
  CPPUNIT_TEST(testCreatePool4);
  CPPUNIT_TEST(testDestroyPool);

  CPPUNIT_TEST(testSubmitEmptyTask);
  CPPUNIT_TEST(testSubmit);
  CPPUNIT_TEST(testSubmitInvalidNumWorkers);
  CPPUNIT_TEST(testSubmitGet);
  CPPUNIT_TEST(testSubmitLoop);
  CPPUNIT_TEST(testSubmitGetLoop);

  CPPUNIT_TEST_SUITE_END();  // VfTasksTest

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
  void testSubmitLoop();
  void testSubmitGetLoop();

  void tearDown();

private:
  void submit_loop();
  int get_loop();

  vftasks_pool_t *pool;  // pointer to a worker-thread pool
  args_t *args;
};

#endif  // VFTASKSTEST_H
