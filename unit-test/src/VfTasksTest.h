// Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.
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

class VfTasksTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(VfTasksTest);
  CPPUNIT_TEST(testExecution);
  CPPUNIT_TEST_SUITE_END();  // VfTasksTest

public:
  void testExecution();

private:
  vftasks_pool_t *pool;  // pointer to a worker-thread pool
};

#endif  // VFTASKSTEST_H
