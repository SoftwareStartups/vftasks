#include "taskstest_busywait.h"

void TasksTestBusyWait::setUp()
{
  TasksTest::setUp();

  this->busy_wait = 1;
}


// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(TasksTestBusyWait);
