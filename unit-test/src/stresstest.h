// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#ifndef STRESS_TEST_H
#define STRESS_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#define N_PARTITIONS 4

extern "C"
{
#include "vftasks.h"
}

class StressTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StressTest);

  CPPUNIT_TEST(testStress);

  CPPUNIT_TEST_SUITE_END(); // StressTest

public:
  void testStress();

  void setUp();
  void tearDown();
};

#endif // STRESS_TEST_H
