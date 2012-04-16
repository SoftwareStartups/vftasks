// Copyright (c) 2012 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#ifndef SYNC_1D_TEST_H
#define SYNC_1D_TEST_H

#include <cppunit/extensions/HelperMacros.h>

extern "C"
{
#include <vftasks.h>
}

class Sync1dTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Sync1dTest);

  CPPUNIT_TEST(testCreateManager);

  CPPUNIT_TEST(testDist1);
  CPPUNIT_TEST(testDist2);
  CPPUNIT_TEST(testDist3);

  CPPUNIT_TEST_SUITE_END(); // Sync1dTest

public:
  void testCreateManager();

  void testDist1();
  void testDist2();
  void testDist3();

  Sync1dTest();

  void setUp();
  void tearDown();

private:
  void testSync(int dist, int index);

  vftasks_1d_sync_mgr_t *sync_mgr;
};

#endif // SYNC_1D_TEST_H
