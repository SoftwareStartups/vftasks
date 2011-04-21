// Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
//
// This file contains proprietary and confidential information of Vector
// Fabrics and all use (including distribution) is subject to the conditions of
// the license agreement between you and Vector Fabrics. Without such a license
// agreement in place, no usage or distribution rights are granted by Vector
// Fabrics.
//
#include "synctest.h"

extern "C"
{
#include <stdlib.h> /* malloc / free */
#include <unistd.h> /* usleep */
#include <pthread.h>
}

#define ROWS 32
#define COLS 32

typedef struct
{
  vftasks_2d_sync_mgr_t *mgr;
  int row; /* row iterator value in which the thread is waiting */
  int col; /* column iterator value in which the thread is waiting */
} args_t;

static pthread_mutex_t mutex;
static volatile int set = 0;

#define ASSERT_AND_CLEAN(mgr,ref)               \
  {                                             \
    CPPUNIT_ASSERT(mgr ref);                    \
    if (mgr != NULL)                            \
      vftasks_destroy_2d_sync_mgr(mgr);         \
  }

/* Thread-safe worker function that sets a global.
 * See testWaitSignal for more info.
 */
static void *waitAndSet(void *raw_args)
{
  int i;
  args_t *args = (args_t *)raw_args;

  for (i = 0; i <= args->col; i++)
    vftasks_wait_2d(args->mgr, args->row, i);

  pthread_mutex_lock(&mutex);
  set = 1;
  pthread_mutex_unlock(&mutex);

  return NULL;
}

/* Thread safe function to read the global.
 * This is used in the main thread to check whether the correct signal has arrived
 * in the waiting thread.
 */
static int get()
{
  int result;

  pthread_mutex_lock(&mutex);
  result = set;
  pthread_mutex_unlock(&mutex);

  return result;
}

void SyncTest::setUp()
{
  pthread_mutex_init(&mutex, NULL);
  set = 0;
}

void SyncTest::tearDown()
{
  if (this->sync_mgr != NULL)
    vftasks_destroy_2d_sync_mgr(this->sync_mgr);

  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
}

void SyncTest::testCreateManager()
{
  this->sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, -1, 1);
  CPPUNIT_ASSERT(this->sync_mgr != NULL);
}

void SyncTest::testCreateManagerBoundaries()
{
  vftasks_2d_sync_mgr_t *sync_mgr;

  sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, ROWS-1, 1);
  ASSERT_AND_CLEAN(sync_mgr, != NULL);
  sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, -1, COLS-1);
  ASSERT_AND_CLEAN(sync_mgr, != NULL);

  sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, ROWS, 1);
  ASSERT_AND_CLEAN(sync_mgr, == NULL);
  sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, -1, COLS);
  ASSERT_AND_CLEAN(sync_mgr, == NULL);

  sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, -ROWS, 1);
  ASSERT_AND_CLEAN(sync_mgr, == NULL);
  sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, -1, -COLS);
  ASSERT_AND_CLEAN(sync_mgr, == NULL);
}

/* Test the wait and wake-through-signal mechanism.
 * This function simulates two workers operating on a 2-dimensional array.
 * A dependency exists with distances rowDist and colDist.
 * The row and col arguments specify the current location in the iteration space
 * on which the worker is waiting.
 *
 * The picture below shows an iteration space of 8x8 with dependency distances of 1 and 1.
 * The current loop iterators are row 2 and col 2.
 * This means that the wait on c can continue until all signal up to d are arrived.
 * This is simulated by generating signals for all complete rows up to the row in which
 * d is present.
 * After that all signals in the dependent row are generated up to one position before the
 * actual dependency d. Throughout these signals, the other worker should still be halted.
 * In this test the worker is asserted to be halted for each signal.
 * After that, one more signal is generated that should wake the worker,
 * which is also asserted.
 *
 *     0   1   2   3   4   5   6   7
 *   +-------------------------------+
 * 0 |   |   |   |   |   |   |   |   |
 *   +-------------------------------+
 * 1 |   |   | d |   |   |   |   |   |
 *   +-----------\-------------------+
 * 2 |   |   |   | c |   |   |   |   |
 *   +-------------------------------+
 * 3 |   |   |   |   |   |   |   |   |
 *   +-------------------------------+
 * 4 |   |   |   |   |   |   |   |   |
 *   +-------------------------------+
 * 5 |   |   |   |   |   |   |   |   |
 *   +-------------------------------+
 * 6 |   |   |   |   |   |   |   |   |
 *   +-------------------------------+
 * 7 |   |   |   |   |   |   |   |   |
 *   +-------------------------------+
 *
 */
void SyncTest::testSync(int rowDist, int colDist, int row, int col)
{
  int i, j;
  args_t args;
  pthread_t thread;

  this->sync_mgr = vftasks_create_2d_sync_mgr(ROWS, COLS, rowDist, colDist);

  /* wait in a separate thread, otherwise the main thread will lock up */
  args.mgr = this->sync_mgr;
  args.row = row;
  args.col = col;
  pthread_create(&thread, NULL, waitAndSet, &args);

  for (i = 0; i < row-rowDist; i++)
    for (j = 0; j < COLS; j++)
    {
      vftasks_signal_2d(this->sync_mgr, i, j);
      usleep(10); // FIXME: not very robust
      CPPUNIT_ASSERT(get() == 0);
    }

  for (j = 0; j < col-colDist; j++)
  {
    vftasks_signal_2d(this->sync_mgr, i, j);
    usleep(10); // FIXME: not very robust
    CPPUNIT_ASSERT(get() == 0);
  }

  vftasks_signal_2d(this->sync_mgr, i, j);
  usleep(10); // FIXME: not very robust
  CPPUNIT_ASSERT(get() == 1);

  pthread_join(thread, NULL);
}

/******************************************************************************
 * positive synchronization tests that do not cross iteration-space boundaries
 ******************************************************************************/

void SyncTest::testPositiveVertical()
{
  /* left column boundary distance 1 */
  this->testSync(1, 0, 1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(1, 0, ROWS-1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(-1, 0, 0, 0);
  this->tearDown();
  this->setUp();
  this->testSync(-1, 0, ROWS-2, 0);
  this->tearDown();

  /* left column boundary distance >1 */
  this->setUp();
  this->testSync(2, 0, 2, 0);
  this->tearDown();
  this->setUp();
  this->testSync(2, 0, 3, 0);
  this->tearDown();
  this->setUp();
  this->testSync(ROWS-1, 0, ROWS-1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(-(ROWS-1), 0, 0, 0);
  this->tearDown();
  this->setUp();
  this->testSync(2, 0, ROWS-1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(-2, 0, ROWS-3, 0);
  this->tearDown();
  this->setUp();
  this->testSync(-2, 0, ROWS-4, 0);
  this->tearDown();

  /* right column boundary distance 1 */
  this->setUp();
  this->testSync(1, 0, 1, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(1, 0, ROWS-1, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(-1, 0, 0, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(-1, 0, ROWS-2, COLS-1);
  this->tearDown();

  /* right column boundary distance >1 */
  this->setUp();
  this->testSync(2, 0, 2, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(2, 0, 3, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(ROWS-1, 0, ROWS-1, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(-(ROWS-1), 0, 0, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(2, 0, ROWS-1, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(-2, 0, ROWS-3, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(-2, 0, ROWS-4, COLS-1);
}

void SyncTest::testPositiveHorizontal()
{
  /* first row boundary distance 1 */
  this->testSync(0, -1, 0, 0);
  this->tearDown();
  this->setUp();
  this->testSync(0, -1, 0, COLS-2);
  this->tearDown();
  this->setUp();
  this->testSync(0, 1, 0, 1);
  this->tearDown();
  this->setUp();
  this->testSync(0, 1, 0, COLS-1);
  this->tearDown();

  /* first row boundary distance >1 */
  this->setUp();
  this->testSync(0, -2, 0, 0);
  this->tearDown();
  this->setUp();
  this->testSync(0, -(COLS-1), 0, 0);
  this->tearDown();
  this->setUp();
  this->testSync(0, -2, 0, COLS-3);
  this->tearDown();
  this->setUp();
  this->testSync(0, -2, 0, COLS-4);
  this->tearDown();
  this->setUp();
  this->testSync(0, 2, 0, 2);
  this->tearDown();
  this->setUp();
  this->testSync(0, 2, 0, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(0, COLS-1, 0, COLS-1);
  this->tearDown();

  /* last row boundary distance 1 */
  this->setUp();
  this->testSync(0, -1, ROWS-1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(0, -1, ROWS-1, COLS-2);
  this->tearDown();
  this->setUp();
  this->testSync(0, 1, ROWS-1, 1);
  this->tearDown();
  this->setUp();
  this->testSync(0, 1, ROWS-1, COLS-1);
  this->tearDown();

  /* last row boundary distance >1 */
  this->setUp();
  this->testSync(0, -2, ROWS-1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(0, -(COLS-1), ROWS-1, 0);
  this->tearDown();
  this->setUp();
  this->testSync(0, -2, ROWS-1, COLS-3);
  this->tearDown();
  this->setUp();
  this->testSync(0, -2, ROWS-1, COLS-4);
  this->tearDown();
  this->setUp();
  this->testSync(0, 2, ROWS-1, 2);
  this->tearDown();
  this->setUp();
  this->testSync(0, 2, ROWS-1, COLS-1);
  this->tearDown();
  this->setUp();
  this->testSync(0, COLS-1, ROWS-1, COLS-1);
}

/******************************************************************************
 * negative synchronization tests
 ******************************************************************************/

CPPUNIT_TEST_SUITE_REGISTRATION(SyncTest);
