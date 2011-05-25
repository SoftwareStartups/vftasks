// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#include "taskstest_busywait.h"

TasksTestBusyWait::TasksTestBusyWait()
{
  TasksTest();

  this->busy_wait = 1;
}


// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(TasksTestBusyWait);
