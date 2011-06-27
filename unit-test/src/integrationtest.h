// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#ifndef INTEGRATION_TEST_H
#define INTEGRATION_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#define N_PARTITIONS 4

extern "C"
{
#include "vftasks.h"
}

typedef struct
{
  int start;
  int stride;
  vftasks_pool_t *pool;
  vftasks_2d_sync_mgr_t *sync_mgr;
} outer_loop_args_t;

typedef struct
{
  int outer_loop_idx;
  int start;
  int stride;
  vftasks_2d_sync_mgr_t *sync_mgr;
} inner_loop_args_t;

class IntegrationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IntegrationTest);

  CPPUNIT_TEST(testDeadlock);

  CPPUNIT_TEST_SUITE_END(); // IntegrationTest

public:
  void testDeadlock();
  //TasksTest();

  void setUp();
  void tearDown();

private:
  vftasks_pool_t *pool;
  vftasks_2d_sync_mgr_t *sync_mgr;
  outer_loop_args_t outer_args[N_PARTITIONS];
};

#endif // INTEGRATION_TEST_H
