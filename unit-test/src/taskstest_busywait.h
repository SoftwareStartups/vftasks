#ifndef TASKSTEST_BUSY_WAIT_H
#define TASKSTEST_BUSY_WAIT_H

#include <cppunit/extensions/HelperMacros.h>

#include "taskstest.h"

class TasksTestBusyWait : public TasksTest
{
  CPPUNIT_TEST_SUITE(TasksTestBusyWait);

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

  CPPUNIT_TEST_SUITE_END();  // TasksTestBusyWait

public:
  void setUp();
};

#endif  // TASKSTEST_BUSY_WAIT_H
