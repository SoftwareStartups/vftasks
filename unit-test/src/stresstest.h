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
