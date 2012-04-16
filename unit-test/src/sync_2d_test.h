// Copyright (c) 2011-2012 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#ifndef SYNC_2D_TEST_H
#define SYNC_2D_TEST_H

#include <cppunit/extensions/HelperMacros.h>

extern "C"
{
#include <vftasks.h>
}

class Sync2dTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Sync2dTest);

  CPPUNIT_TEST(testCreateManager);
  CPPUNIT_TEST(testCreateManagerBoundaries);

  CPPUNIT_TEST(testVertical);
  CPPUNIT_TEST(testHorizontal);
  CPPUNIT_TEST(testDiagonal);
  CPPUNIT_TEST(testBorderCrossing);

  CPPUNIT_TEST_SUITE_END(); // Sync2dTest

public:
  void testCreateManager();
  void testCreateManagerBoundaries();

  void testVertical();
  void testHorizontal();
  void testDiagonal();
  void testBorderCrossing();

  Sync2dTest();

  void setUp();
  void tearDown();

private:
  void testSync(int rowDist, int colDist, int row, int col);
  void testNoSync(int rowDist, int colDist, int row, int col);

  vftasks_2d_sync_mgr_t *sync_mgr;
};

#endif // SYNC_2D_TEST_H
