// Copyright (c) 2012 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#include "sync_1d_test.h"

extern "C"
{
#include <stdlib.h> /* malloc / free */
#include "platform.h"
}

#define N 32

typedef struct
{
  vftasks_1d_sync_mgr_t *mgr;
  int index; /* iteration in which the thread is waiting */
} args_t;

static semaphore_t sem;
static volatile int set = 0;

#define ASSERT_AND_CLEAN(mgr,ref)               \
  {                                             \
    CPPUNIT_ASSERT(mgr ref);                    \
    if (mgr != NULL)                            \
      vftasks_destroy_1d_sync_mgr(mgr);         \
  }

/* Thread-safe worker function that sets a global.
 * See testWaitSignal for more info.
 */
static WORKER_PROTO(waitAndSet, raw_args)
{
  int i;
  args_t *args = (args_t *)raw_args;

  for (i = 0; i <= args->index; i++)
    vftasks_wait_1d(args->mgr, i);

  set = 1;
  SEMAPHORE_POST(sem);

  return NULL;
}

/* Thread safe function to read the global.
 * This is used in the main thread to check whether the correct signal has arrived
 * in the waiting thread.
 */
static int get()
{
  SEMAPHORE_WAIT(sem);
  return set;
}

Sync1dTest::Sync1dTest()
{
  this->sync_mgr = NULL;
}

void Sync1dTest::setUp()
{
  SEMAPHORE_CREATE(sem, 0, 1);
  set = 0;
}

void Sync1dTest::tearDown()
{
  if (this->sync_mgr != NULL)
    vftasks_destroy_1d_sync_mgr(this->sync_mgr);

  SEMAPHORE_DESTROY(sem);
}

void Sync1dTest::testCreateManager()
{
  this->sync_mgr = vftasks_create_1d_sync_mgr(2, 1);
  CPPUNIT_ASSERT(this->sync_mgr != NULL);
}

/* Test the wait and wake-through-signal mechanism.
 * This function simulates two workers operating on a single array.
 * A dependency exists with distance dist.
 * The index argument specifies the current location in the iteration space on which the
 * the worker is waiting.
 *
 * The picture below shows an iteration space of 8 with a dependency distance of 2.
 * The current loop index is 3.
 * This means that the wait on c lasts until d has signalled.
 *
 *   0   1   2   3   4   5   6   7
 * +-------------------------------+
 * |   | d-+---+-c |   |   |   |   |
 * +-------------------------------+
 *
 */
void Sync1dTest::testSync(int dist, int index)
{
  int i;
  args_t args;
  thread_t thread;

  this->sync_mgr = vftasks_create_1d_sync_mgr(2, dist);

  /* wait in a separate thread, otherwise the main thread will lock up */
  args.mgr = this->sync_mgr;
  args.index = index;
  THREAD_CREATE(thread, waitAndSet, &args);

  for (i = 0; i < index - dist; i++)
  {
    vftasks_signal_1d(this->sync_mgr, i);
    CPPUNIT_ASSERT(set == 0);
  }

  vftasks_signal_1d(this->sync_mgr, index - dist);
  CPPUNIT_ASSERT(get() == 1);

  THREAD_JOIN(thread);
}

/******************************************************************************
 * synchronization tests
 ******************************************************************************/

void Sync1dTest::testDist1()
{
  this->testSync(1, 1);
  this->tearDown();
  this->setUp();
  this->testSync(1, 2);
  this->tearDown();
  this->setUp();
  this->testSync(1, 3);
}

void Sync1dTest::testDist2()
{
  this->testSync(2, 1);
  this->tearDown();
  this->setUp();
  this->testSync(2, 2);
  this->tearDown();
  this->setUp();
  this->testSync(2, 3);
}

void Sync1dTest::testDist3()
{
  this->testSync(3, 1);
  this->tearDown();
  this->setUp();
  this->testSync(3, 2);
  this->tearDown();
  this->setUp();
  this->testSync(3, 3);
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(Sync1dTest);
