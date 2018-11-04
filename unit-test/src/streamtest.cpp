#include "streamtest.h"

#include <limits>   // for numeric_limits
#include <cmath>    // for fabs
#include <cstdlib>  // for malloc, free

extern "C"
{
#include "platform.h"
}

#ifdef _POSIX_SOURCE
#include <pthread.h>
#define EXEC_ON_WORKER_THREAD(FUNC,ARG)                                 \
  {                                                                     \
    pthread_t worker;                                                   \
    void *result;                                                       \
    CPPUNIT_ASSERT(pthread_create(&worker, NULL, FUNC, ARG) == 0);      \
    CPPUNIT_ASSERT(pthread_join(worker, &result) == 0);                 \
  }
#elif defined(_WIN32)
#include <windows.h>
#define EXEC_ON_WORKER_THREAD(FUNC,ARG)                         \
  {                                                             \
    DWORD exit_code;                                            \
    HANDLE worker = CreateThread(NULL, 0, FUNC, ARG, 0, NULL);  \
    WaitForSingleObject(worker, INFINITE);                      \
    GetExitCodeThread(worker, &exit_code);                      \
    CloseHandle(worker);                                        \
    CPPUNIT_ASSERT(exit_code == 0);                             \
  }
#else
#error("unsupported platform")
#endif


#define EQ(TYPE, X, Y)   (fabs(X - Y) < std::numeric_limits<TYPE>::epsilon())
#define EQ_FLOAT(X, Y)   EQ(float, X, Y)
#define EQ_DOUBLE(X, Y)  EQ(double, X, Y)

#define CREATE_CHAN(NUM_TOKENS, TOKEN_SIZE)                             \
  this->chan = vftasks_create_chan(NUM_TOKENS,                          \
                                   TOKEN_SIZE,                          \
                                   this->mem_mgr,                       \
                                   this->mem_mgr);                      \
  CPPUNIT_ASSERT(this->chan != NULL);
#define WITH_WPORT                                                     \
  this->wport = vftasks_create_write_port(this->chan, this->mem_mgr);  \
  CPPUNIT_ASSERT(this->wport != NULL);
#define WITH_RPORT                                                     \
  this->rport = vftasks_create_read_port(this->chan, this->mem_mgr);   \
  CPPUNIT_ASSERT(this->rport != NULL);

#define WPORT_FROM_VOID(WPORT, VOID)                \
  vftasks_wport_t *WPORT = (vftasks_wport_t *)VOID;
#define RPORT_FROM_VOID(RPORT, VOID)                \
  vftasks_rport_t *RPORT = (vftasks_rport_t *)VOID;


// default channel hooks
static void suspendWriter(vftasks_wport_t *wport)
{
}

static void resumeWriter(vftasks_wport_t *wport)
{
}

static void suspendReader(vftasks_rport_t *rport)
{
}

static void resumeReader(vftasks_rport_t *rport)
{
}


void StreamTest::increaseWriterSuspendCount()
{
  ++this->writerSuspendCount;
}

int StreamTest::getWriterSuspendCount()
{
  return this->writerSuspendCount;
}

void StreamTest::increaseWriterResumeCount()
{
  ++this->writerResumeCount;
}

int StreamTest::getWriterResumeCount()
{
  return this->writerResumeCount;
}

void StreamTest::increaseReaderSuspendCount()
{
  ++this->readerSuspendCount;
}

int StreamTest::getReaderSuspendCount()
{
  return this->readerSuspendCount;
}

void StreamTest::increaseReaderResumeCount()
{
  ++this->readerResumeCount;
}

int StreamTest::getReaderResumeCount()
{
  return this->readerResumeCount;
}

void StreamTest::setUp()
{
  // allocate space for memory manager
  this->mem_mgr = (vftasks_malloc_t *)malloc(sizeof(vftasks_malloc_t));
  CPPUNIT_ASSERT(this->mem_mgr != NULL);

  // initialize memory manager
  this->mem_mgr->malloc = malloc;
  this->mem_mgr->free = free;

  // initialize channel and ports
  this->chan = NULL;
  this->wport = NULL;
  this->rport = NULL;

  // initialize suspend and resume counts
  this->writerSuspendCount = 0;
  this->writerResumeCount = 0;
  this->readerSuspendCount = 0;
  this->readerResumeCount = 0;
}

void StreamTest::tearDown()
{
  // destroy ports and channel
  if (this->wport != NULL)
    vftasks_destroy_write_port(this->wport, this->mem_mgr);
  if (this->rport != NULL)
    vftasks_destroy_read_port(this->rport, this->mem_mgr);
  if (this->chan != NULL)
    vftasks_destroy_chan(this->chan, this->mem_mgr, this->mem_mgr);

  // free space for memory manager
  if (this->mem_mgr != NULL) free(this->mem_mgr);
}

void StreamTest::testCreation()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // verify channel queries
  CPPUNIT_ASSERT(vftasks_get_num_tokens(this->chan) == 16);
  CPPUNIT_ASSERT(vftasks_get_token_size(this->chan) == 8);

  // verify port queries
  CPPUNIT_ASSERT(vftasks_chan_of_wport(this->wport) == this->chan);
  CPPUNIT_ASSERT(vftasks_chan_of_rport(this->rport) == this->chan);
}

void StreamTest::testCreatingTokenlessChannel()
{
  // try to create channel and verify that it failed
  this->chan = vftasks_create_chan(0, 8, this->mem_mgr, this->mem_mgr);
  CPPUNIT_ASSERT(this->chan == NULL);
}

void StreamTest::testCreatingChannelWithNegativeNumberOfTokens()
{
  // try to create channel and verify that it failed
  this->chan = vftasks_create_chan(-1, 8, this->mem_mgr, this->mem_mgr);
  CPPUNIT_ASSERT(this->chan == NULL);
}

void StreamTest::testCreatingChannelWithZeroSizeTokens()
{
  // try to create channel and verify that it failed
  this->chan = vftasks_create_chan(16, 0, this->mem_mgr, this->mem_mgr);
  CPPUNIT_ASSERT(this->chan == NULL);
}

void StreamTest::testRoundingUpTokenSize()
{
  CREATE_CHAN(16, 51);

  // verify token size
  CPPUNIT_ASSERT(vftasks_get_token_size(this->chan) == 64);
}

void StreamTest::testConnectingMultipleWritePorts()
{
  CREATE_CHAN(16, 8) WITH_WPORT;

  // verify that connecting a second write port fails
  CPPUNIT_ASSERT(vftasks_create_write_port(this->chan, this->mem_mgr) == NULL);
}

void StreamTest::testConnectingMultipleReadPorts()
{
  CREATE_CHAN(16, 8) WITH_RPORT;

  // verify that connecting a second read port fails
  CPPUNIT_ASSERT(vftasks_create_read_port(this->chan, this->mem_mgr) == NULL);
}

void StreamTest::testWritePortRenewal()
{
  CREATE_CHAN(16, 8) WITH_WPORT;

  // destroy port
  vftasks_destroy_write_port(this->wport, this->mem_mgr);

  // verify that a new write port can be connected now
  this->wport = vftasks_create_write_port(this->chan, this->mem_mgr);
  CPPUNIT_ASSERT(this->wport != NULL);
}

void StreamTest::testReadPortRenewal()
{
  CREATE_CHAN(16, 8) WITH_RPORT;

  // destroy port
  vftasks_destroy_read_port(this->rport, this->mem_mgr);

  // verify that a new read port can be connected now
  this->rport = vftasks_create_read_port(this->chan, this->mem_mgr);
  CPPUNIT_ASSERT(this->rport != NULL);
}

void StreamTest::testInitialLowWaterMark()
{
  CREATE_CHAN(16, 8);

  // verify that the initial low-water mark is 1
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 1);
}

void StreamTest::testSettingLowWaterMark()
{
  CREATE_CHAN(16, 8);

  // set the low-water mark and verify its new value
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, 7) == 7);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 7);
}

void StreamTest::testMinimizingLowWaterMark()
{
  CREATE_CHAN(16, 8);

  // by default the low-water mark is already minimized; here, we give it a
  // temporary value first, reminimize it, and then verify its new value
  vftasks_set_min_room(this->chan, 2);
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, 1) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 1);
}

void StreamTest::testSettingLowWaterMarkTooLow()
{
  CREATE_CHAN(16, 8);

  // set the low-water mark too low and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, 0) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 1);
}

void StreamTest::testSettingLowWaterMarkWayTooLow()
{
  CREATE_CHAN(16, 8);

  // set the low-water mark way too low and that verify it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, -5) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 1);
}

void StreamTest::testMaximizingLowWaterMark()
{
  CREATE_CHAN(16, 8);

  // maximize the low-water mark and verify its new value
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, 16) == 16);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 16);
}

void StreamTest::testSettingLowWaterMarkTooHigh()
{
  CREATE_CHAN(16, 8);

  // set the low-water mark too high and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, 17) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 1);
}

void StreamTest::testSettingLowWaterMarkWayTooHigh()
{
  CREATE_CHAN(16, 8);

  // set the low-water mark way too high and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_room(this->chan, 19) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_room(this->chan) == 1);
}

void StreamTest::testInitialHighWaterMark()
{
  CREATE_CHAN(16,  8);

  // verify that the initial high-water mark is 1
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 1);
}

void StreamTest::testSettingHighWaterMark()
{
  CREATE_CHAN(16, 8);

  // set the high-water mark and verify its new value
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, 7) == 7);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 7);
}

void StreamTest::testMinimizingHighWaterMark()
{
  CREATE_CHAN(16, 8);

  // by default the high-water mark is already minimized; here, we give it a
  // temporary value, reminimize it, and then verify its new value
  vftasks_set_min_data(this->chan, 2);
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, 1) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 1);
}

void StreamTest::testSettingHighWaterMarkTooLow()
{
  CREATE_CHAN(16, 8);

  // set the high-water mark too low and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, 0) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 1);
}

void StreamTest::testSettingHighWaterMarkWayTooLow()
{
  CREATE_CHAN(16, 8);

  // set the high-water mark way too low and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, -5) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 1);
}

void StreamTest::testMaximizingHighWaterMark()
{
  CREATE_CHAN(16, 8);

  // maximize the high-water mark and verify its new value
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, 16) == 16);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 16);
}

void StreamTest::testSettingHighWaterMarkTooHigh()
{
  CREATE_CHAN(16, 8);

  // set the high-water mark too high and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, 17) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 1);
}

void StreamTest::testSettingHighWaterMarkWayTooHigh()
{
  CREATE_CHAN(16, 8);

  // set the high-water mark way too high and verify that it's not updated
  CPPUNIT_ASSERT(vftasks_set_min_data(this->chan, 19) == 1);
  CPPUNIT_ASSERT(vftasks_get_min_data(this->chan) == 1);
}

void StreamTest::testInitialApplicationSpecificData()
{
  CREATE_CHAN(16, 8);

  // verify that initially there is no application-specific data
  CPPUNIT_ASSERT(vftasks_get_chan_info(this->chan) == NULL);
}

void StreamTest::testSettingApplicationSpecificData()
{
  CREATE_CHAN(16, 8);

  // set the application-specific data and verify its new value
  vftasks_set_chan_info(this->chan, this);
  CPPUNIT_ASSERT(vftasks_get_chan_info(this->chan) == this);
}

void StreamTest::testRetrievingChannelFromWritePort()
{
  CREATE_CHAN(16, 8) WITH_WPORT;

  // verify channel of port
  CPPUNIT_ASSERT(vftasks_chan_of_wport(this->wport) == this->chan);
}

void StreamTest::testRetrievingChannelFromReadPort()
{
  CREATE_CHAN(16, 8) WITH_RPORT;

  // verify channel of port
  CPPUNIT_ASSERT(vftasks_chan_of_rport(this->rport) == this->chan);
}

void StreamTest::testInitialRoomAvailable()
{
  CREATE_CHAN(16, 8) WITH_WPORT;

  // verify that initially there is room available
  CPPUNIT_ASSERT(vftasks_room_available(this->wport));
}

void StreamTest::testRoomAvailableAfterWriting()
{
  CREATE_CHAN(16, 8) WITH_WPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // verify that there is still room available
  CPPUNIT_ASSERT(vftasks_room_available(this->wport));
}

void StreamTest::testRoomAvailableAfterFilling()
{
  CREATE_CHAN(16, 8) WITH_WPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // verify that there is no more room available
  CPPUNIT_ASSERT(!vftasks_room_available(this->wport));
}

void StreamTest::testRoomAvailableAfterWritingAndReading()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // verify that there is still room available
  CPPUNIT_ASSERT(vftasks_room_available(this->wport));
}

void StreamTest::testRoomAvailableAfterFillingAndReading()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // verify that there is again room available
  CPPUNIT_ASSERT(vftasks_room_available(this->wport));
}

void StreamTest::testRoomAvailableAfterWritingAndEmptying()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // empty the channel
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // verify that there is still room available
  CPPUNIT_ASSERT(vftasks_room_available(this->wport));
}


void StreamTest::testRoomAvailableAfterFillingAndEmptying()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // empty the channel
  for (int j = 16; j > 0; --j) vftasks_read_int32(this->rport);

  // verify that there is again room available
  CPPUNIT_ASSERT(vftasks_room_available(this->wport));
}

void StreamTest::testInitialDataAvailable()
{
  CREATE_CHAN(16, 8) WITH_RPORT;

  // verify that initially there is no data available
  CPPUNIT_ASSERT(!vftasks_data_available(this->rport));
}


void StreamTest::testDataAvailableAfterWriting()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // verify that there is now data available
  CPPUNIT_ASSERT(vftasks_data_available(this->rport));
}

void StreamTest::testDataAvailableAfterFilling()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // verify that there is now data available
  CPPUNIT_ASSERT(vftasks_data_available(this->rport));
}

void StreamTest::testDataAvailableAfterWritingAndReading()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // verify that there is still data available
  CPPUNIT_ASSERT(vftasks_data_available(this->rport));
}

void StreamTest::testDataAvailableAfterFillingAndReading()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // verify that there is still data available
  CPPUNIT_ASSERT(vftasks_data_available(this->rport));
}

void StreamTest::testDataAvailableAfterWritingAndEmptying()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // empty the channel
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // verify that there is no more data available
  CPPUNIT_ASSERT(!vftasks_data_available(this->rport));
}


void StreamTest::testDataAvailableAfterFillingAndEmptying()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // empty the channel
  for (int j = 16; j > 0; --j) vftasks_read_int32(this->rport);

  // verify that there is no more data available
  CPPUNIT_ASSERT(!vftasks_data_available(this->rport));
}

void StreamTest::testAcquiringRoomInitially()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT;

  // acquire room
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token != NULL);

  // release data
  vftasks_release_data(this->wport, token);
}

void StreamTest::testAcquiringRoomAfterWriting()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // acquire room
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token != NULL);

  // release data
  vftasks_release_data(this->wport, token);
}

void StreamTest::testAcquiringRoomAfterFilling()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // try to acquire room and verify that it failed
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token == NULL);
}

void StreamTest::testAcquiringRoomAfterWritingAndReading()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // acquire room
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token != NULL);

  // release data
  vftasks_release_data(this->wport, token);
}

void StreamTest::testAcquiringRoomAfterFillingAndReading()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // acquire room
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token != NULL);

  // release data
  vftasks_release_data(this->wport, token);
}

void StreamTest::testAcquiringRoomAfterWritingAndEmptying()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // empty the channel
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // acquire room
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token != NULL);

  // release data
  vftasks_release_data(this->wport, token);
}

void StreamTest::testAcquiringRoomAfterFillingAndEmptying()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // empty the channel
  for (int j = 16; j > 0; --j) vftasks_read_int32(this->rport);

  // acquire room
  token = vftasks_acquire_room_nb(this->wport);
  CPPUNIT_ASSERT(token != NULL);

  // release data
  vftasks_release_data(this->wport, token);
}

void StreamTest::testAcquiringDataInitially()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_RPORT;

  // try to acquire data and verify that it failed
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token == NULL);
}

void StreamTest::testAcquiringDataAfterWriting()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // acquire data
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token != NULL);

  // release room
  vftasks_release_room(this->rport, token);
}

void StreamTest::testAcquiringDataAfterFilling()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // acquire data
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token != NULL);

  // release room
  vftasks_release_room(this->rport, token);
}

void StreamTest::testAcquiringDataAfterWritingAndReading()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // acquire data
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token != NULL);

  // release room
  vftasks_release_room(this->rport, token);
}

void StreamTest::testAcquiringDataAfterFillingAndReading()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // read some tokens
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // acquire data
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token != NULL);

  // release room
  vftasks_release_room(this->rport, token);
}

void StreamTest::testAcquiringDataAfterWritingAndEmptying()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some tokens
  vftasks_write_int32(this->wport, 2);
  vftasks_write_int32(this->wport, 3);
  vftasks_write_int32(this->wport, 5);

  // empty the channel
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);
  vftasks_read_int32(this->rport);

  // try to acquire room and verify that it failed
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token == NULL);
}

void StreamTest::testAcquiringDataAfterFillingAndEmptying()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // empty the channel
  for (int j = 16; j > 0; --j) vftasks_read_int32(this->rport);

  // try to acquire data and verify that it failed
  token = vftasks_acquire_data_nb(this->rport);
  CPPUNIT_ASSERT(token == NULL);
}

void StreamTest::testSharedMemorySupport()
{
  CREATE_CHAN(16, 8);

  // verify shared-memory support
  CPPUNIT_ASSERT(vftasks_shmem_supported(this->chan));
}

void StreamTest::testSharedMemoryMode()
{
  vftasks_token_t *token;  // pointer to a token
  char *wptr;             // pointer into a room token
  char *rptr;             // pointer into a data token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // acquire room
  token = vftasks_acquire_room(this->wport);

  // get pointer into the token
  wptr = (char *)vftasks_get_memaddr(token);
  CPPUNIT_ASSERT(wptr != NULL);

  // write some data into the token
  *(int32_t *)wptr = 2;
  *(int16_t *)(wptr + 4) = 3;
  *(int8_t *)(wptr + 7) = 5;

  // release data
  vftasks_release_data(this->wport, token);

  // acquire data
  token = vftasks_acquire_data(this->rport);

  // get pointer into the token and verify the address it points to
  rptr = (char *)vftasks_get_memaddr(token);
  CPPUNIT_ASSERT(rptr == wptr);

  // read data from the token
  CPPUNIT_ASSERT(*(int32_t *)wptr == 2);
  CPPUNIT_ASSERT(*(int16_t *)(wptr + 4) == 3);
  CPPUNIT_ASSERT(*(int8_t *)(wptr + 7) == 5);

  // release room
  vftasks_release_room(this->rport, token);
}

void StreamTest::testFifoBehaviorInSharedMemoryMode()
{
  vftasks_token_t *token;  // pointer to a token
  char *ptr;              // pointer into a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write first token
  token = vftasks_acquire_room(this->wport);
  ptr = (char *)vftasks_get_memaddr(token);
  *(int32_t *)ptr = 2;
  vftasks_release_data(this->wport, token);

  // write second token
  token = vftasks_acquire_room(this->wport);
  ptr = (char *)vftasks_get_memaddr(token);
  *(int32_t *)ptr = 3;
  vftasks_release_data(this->wport, token);

  // read first token
  token = vftasks_acquire_data(this->rport);
  ptr = (char *)vftasks_get_memaddr(token);
  CPPUNIT_ASSERT(*(int32_t *)ptr == 2);
  vftasks_release_room(this->rport, token);

  // write third token
  token = vftasks_acquire_room(this->wport);
  ptr = (char *)vftasks_get_memaddr(token);
  *(int32_t *)ptr = 5;
  vftasks_release_data(this->wport, token);

  // read second token
  token = vftasks_acquire_data(this->rport);
  ptr = (char *)vftasks_get_memaddr(token);
  CPPUNIT_ASSERT(*(int32_t *)ptr == 3);
  vftasks_release_room(this->rport, token);

  // read third token
  token = vftasks_acquire_data(this->rport);
  ptr = (char *)vftasks_get_memaddr(token);
  CPPUNIT_ASSERT(*(int32_t *)ptr == 5);
  vftasks_release_room(this->rport, token);
}

void StreamTest::testTokenReuseInSharedMemoryMode()
{
  vftasks_token_t *token;  // pointer to a token
  char *ptr;              // pointer into a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i)
  {
    token = vftasks_acquire_room(this->wport);
    ptr = (char *)vftasks_get_memaddr(token);
    *(int32_t *)ptr = i;
    vftasks_release_data(this->wport, token);
  }

  // read half of the channel
  for (int j = 0; j < 8; ++j)
  {
    token = vftasks_acquire_data(this->rport);
    ptr = (char *)vftasks_get_memaddr(token);
    CPPUNIT_ASSERT(*(int32_t *)ptr == j);
    vftasks_release_room(this->rport, token);
  }

  // refill the channel
  for (int i = 16; i < 24; ++i)
  {
    token = vftasks_acquire_room(this->wport);
    ptr = (char *)vftasks_get_memaddr(token);
    *(int32_t *)ptr = i;
    vftasks_release_data(this->wport, token);
  }

  // read the entire channel
  for (int j = 8; j < 24; ++j)
  {
    token = vftasks_acquire_data(this->rport);
    ptr = (char *)vftasks_get_memaddr(token);
    CPPUNIT_ASSERT(*(int32_t *)ptr == j);
    vftasks_release_room(this->rport, token);
  }
}

void StreamTest::testWindowedMode()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // acquire room
  token = vftasks_acquire_room(this->wport);

  // write some data into the token
  vftasks_put_int32(token, 0, 2);
  vftasks_put_int16(token, 4, 3);
  vftasks_put_int8(token, 7, 5);

  // release data
  vftasks_release_data(this->wport, token);

  // acquire data
  token = vftasks_acquire_data(this->rport);

  // read data from the token
  CPPUNIT_ASSERT(vftasks_get_int32(token, 0) == 2);
  CPPUNIT_ASSERT(vftasks_get_int16(token, 4) == 3);
  CPPUNIT_ASSERT(vftasks_get_int8(token, 7) == 5);

  // release room
  vftasks_release_room(this->rport, token);
}

void StreamTest::testFifoBehaviorInWindowedMode()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write first token
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int32(token, 0, 2);
  vftasks_release_data(this->wport, token);

  // write second token
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int32(token, 0, 3);
  vftasks_release_data(this->wport, token);

  // read first token
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int32(token, 0) == 2);
  vftasks_release_room(this->rport, token);

  // write third token
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int32(token, 0, 5);
  vftasks_release_data(this->wport, token);

  // read second token
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int32(token, 0) == 3);
  vftasks_release_room(this->rport, token);

  // read third token
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int32(token, 0) == 5);
  vftasks_release_room(this->rport, token);
}

void StreamTest::testTokenReuseInWindowedMode()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i)
  {
    token = vftasks_acquire_room(this->wport);
    vftasks_put_int32(token, 0, i);
    vftasks_release_data(this->wport, token);
  }

  // read half of the channel
  for (int j = 0; j < 8; ++j)
  {
    token = vftasks_acquire_data(this->rport);
    CPPUNIT_ASSERT(vftasks_get_int32(token, 0) == j);
    vftasks_release_room(this->rport, token);
  }

  // refill the channel
  for (int i = 16; i < 24; ++i)
  {
    token = vftasks_acquire_room(this->wport);
    vftasks_put_int32(token, 0, i);
    vftasks_release_data(this->wport, token);
  }

  // read the entire channel
  for (int j = 8; j < 24; ++j)
  {
    token = vftasks_acquire_data(this->rport);
    CPPUNIT_ASSERT(vftasks_get_int32(token, 0) == j);
    vftasks_release_room(this->rport, token);
  }
}

void StreamTest::testKahnMode()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write some data
  vftasks_write_int16(this->wport, 11);

  // read data
  CPPUNIT_ASSERT(vftasks_read_int16(this->rport) == 11);
}

void StreamTest::testFifoBehaviorInKahnMode()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write first token
  vftasks_write_int32(this->wport, 2);

  // write second token
  vftasks_write_int32(this->wport, 3);

  // read first token
  CPPUNIT_ASSERT(vftasks_read_int32(this->rport) == 2);

  // write third token
  vftasks_write_int32(this->wport, 5);

  // read second token
  CPPUNIT_ASSERT(vftasks_read_int32(this->rport) == 3);

  // read third token
  CPPUNIT_ASSERT(vftasks_read_int32(this->rport) == 5);
}

void StreamTest::testTokenReuseInKahnMode()
{
  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(this->wport, i);

  // read half of the channel
  for (int j = 0; j < 8; ++j)
    CPPUNIT_ASSERT(vftasks_read_int32(this->rport) == j);

  // refill the channel
  for (int i = 16; i < 24; ++i) vftasks_write_int32(this->wport, i);

  // read the entire channel
  for (int j = 8; j < 24; ++j)
    CPPUNIT_ASSERT(vftasks_read_int32(this->rport) == j);
}

void StreamTest::testWrappingPutOffsets()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write data at under- and overflowing offsets
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int8(token, -1, 2);
  vftasks_put_int16(token, 8, 3);
  vftasks_put_int32(token, 10, 5);
  vftasks_put_int8(token, 22, 7);
  vftasks_release_data(this->wport, token);

  // read data
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int16(token, 0) == 3);
  CPPUNIT_ASSERT(vftasks_get_int32(token, 2) == 5);
  CPPUNIT_ASSERT(vftasks_get_int8(token, 6) == 7);
  CPPUNIT_ASSERT(vftasks_get_int8(token, 7) == 2);
  vftasks_release_room(this->rport, token);
}

void StreamTest::testWrappingGetOffsets()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write data
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int8(token, 0, 2);
  vftasks_put_int16(token, 1, 3);
  vftasks_put_int32(token, 3, 5);
  vftasks_put_int8(token, 7, 7);
  vftasks_release_data(this->wport, token);

  // read data from under- and overflowing offsets
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int8(token, -1) == 7);
  CPPUNIT_ASSERT(vftasks_get_int8(token, 8) == 2);
  CPPUNIT_ASSERT(vftasks_get_int16(token, 9) == 3);
  CPPUNIT_ASSERT(vftasks_get_int32(token, 19) == 5);
  vftasks_release_room(this->rport, token);
}

void StreamTest::testOverflow()
{
  vftasks_token_t *token;  // pointer to a token

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // write a partially overflowing 16-bit integer
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int16(token, 7, 2);
  vftasks_release_data(this->wport, token);

  // write a partially overflowing 32-bit integer
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int32(token, 6, 3);
  vftasks_release_data(this->wport, token);

  // write a partially overflowing 64-bit integer
  token = vftasks_acquire_room(this->wport);
  vftasks_put_int64(token, 4, 5);
  vftasks_release_data(this->wport, token);

  // write a partially overflowing single-precision float
  token = vftasks_acquire_room(this->wport);
  vftasks_put_float(token, 7, 1.62);
  vftasks_release_data(this->wport, token);

  // write a partially overflowing double-precision float
  token = vftasks_acquire_room(this->wport);
  vftasks_put_double(token, 7, 3.14);
  vftasks_release_data(this->wport, token);

  // write a partially overflowing pointer
  token = vftasks_acquire_room(this->wport);
  vftasks_put_ptr(token, 7, this);
  vftasks_release_data(this->wport, token);

  // read the 16-bit integer
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int16(token, 7) == 2);
  vftasks_release_room(this->rport, token);

  // read the 32-bit integer
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int32(token, 6) == 3);
  vftasks_release_room(this->rport, token);

  // read the 64-bit integer
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_int64(token, 4) == 5);
  vftasks_release_room(this->rport, token);

  // read the single-precision float
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(EQ_FLOAT(vftasks_get_float(token, 7), 1.62));
  vftasks_release_room(this->rport, token);

  // read the double-precision float
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(EQ_DOUBLE(vftasks_get_double(token, 7), 3.14));
  vftasks_release_room(this->rport, token);

  // read the pointer
  token = vftasks_acquire_data(this->rport);
  CPPUNIT_ASSERT(vftasks_get_ptr(token, 7) == this);
  vftasks_release_room(this->rport, token);
}

static WORKER_PROTO(testSuspendingWriter_write, arg)
{
  WPORT_FROM_VOID(wport, arg);

  // write a token
  vftasks_write_int32(wport, 2);

  // try to write another token; this should cause the writer to be suspended
  vftasks_write_int32(wport, 3);

  // exit writer thread
  return THREAD_EXIT_SUCCESS;
}

static void testSuspendingWriter_suspendWriter(vftasks_wport_t *wport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_wport(wport)) = true;

  // exit writer thread
  THREAD_EXIT();
}

void StreamTest::testSuspendingWriter()
{
  bool flag;  // flag

  CREATE_CHAN(1, 8) WITH_WPORT WITH_RPORT;

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             testSuspendingWriter_suspendWriter,
                             resumeWriter,
                             suspendReader,
                             resumeReader);

  // intialize flag and add it to channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testSuspendingWriter_write, this->wport);

  // verify that writer was suspended
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testResumingWriter_write, arg)
{
  WPORT_FROM_VOID(wport, arg);

  // write a token
  vftasks_write_int32(wport, 2);

  // try to write another token; this should cause the writer to be suspended
  vftasks_write_int32(wport, 3);

  // exit writer thread
  return THREAD_EXIT_SUCCESS;
}

static void testResumingWriter_suspendWriter(vftasks_wport_t *wport)
{
  // exit writer thread
  THREAD_EXIT();
}

static void testResumingWriter_resumeWriter(vftasks_wport_t *wport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_wport(wport)) = true;
}

void StreamTest::testResumingWriter()
{
  bool flag;

  CREATE_CHAN(1, 8) WITH_WPORT WITH_RPORT;

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             testResumingWriter_suspendWriter,
                             testResumingWriter_resumeWriter,
                             suspendReader,
                             resumeReader);

  // initialize flag and add to it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testResumingWriter_write, this->wport);

  // read a token
  vftasks_read_int32(this->rport);

  // verify that writer has resumed
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testSuspendingReader_read, arg)
{
  RPORT_FROM_VOID(rport, arg);

  // try to read a token; this should cause the reader to be suspended
  vftasks_read_int32(rport);

  // exit writer thread
  return THREAD_EXIT_SUCCESS;
}

static void testSuspendingReader_suspendReader(vftasks_rport_t *rport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_rport(rport)) = true;

  // exit writer thread
  THREAD_EXIT();
}

void StreamTest::testSuspendingReader()
{
  bool flag;  // flag

  CREATE_CHAN(1, 8) WITH_WPORT WITH_RPORT;

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             suspendWriter,
                             resumeWriter,
                             testSuspendingReader_suspendReader,
                             resumeReader);

  // initialize flag and add it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testSuspendingReader_read, this->rport);

  // verify that reader was suspended
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testResumingReader_read, arg)
{
  RPORT_FROM_VOID(rport, arg);

  // try to read a token; this should cause the reader to be suspended
  vftasks_read_int32(rport);

  // exit reader thread
  return THREAD_EXIT_SUCCESS;
}

static void testResumingReader_suspendReader(vftasks_rport_t *rport)
{
  // exit reader thread
  THREAD_EXIT();
}

static void testResumingReader_resumeReader(vftasks_rport_t *rport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_rport(rport)) = true;
}

void StreamTest::testResumingReader()
{
  bool flag;

  CREATE_CHAN(1, 8) WITH_WPORT WITH_RPORT;

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             suspendWriter,
                             resumeWriter,
                             testResumingReader_suspendReader,
                             testResumingReader_resumeReader);

  // initialize flag and add it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testResumingReader_read, this->rport);

  // write a token
  vftasks_write_int32(this->wport, 2);

  // verify that reader has resumed
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testHittingLowWaterMark_write, arg)
{
  WPORT_FROM_VOID(wport, arg);

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(wport, i);

  // try to write another token; this should cause the writer to be suspended
  vftasks_write_int32(wport, 17);

  // exit writer thread
  return THREAD_EXIT_SUCCESS;
}

static void testHittingLowWaterMark_suspendWriter(vftasks_wport_t *wport)
{
  // exit writer thread
  THREAD_EXIT();
}

static void testHittingLowWaterMark_resumeWriter(vftasks_wport_t *wport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_wport(wport)) = true;
}

void StreamTest::testHittingLowWaterMark()
{
  bool flag;  // flag

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // set low-water mark
  vftasks_set_min_room(this->chan, 6);

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             testHittingLowWaterMark_suspendWriter,
                             testHittingLowWaterMark_resumeWriter,
                             suspendReader,
                             resumeReader);

  // initialize flag and add it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testHittingLowWaterMark_write, this->wport);

  // read five tokens
  for (int j = 0; j < 5; ++j) vftasks_read_int32(this->rport);

  // verify that writer hasn't resumed yet
  CPPUNIT_ASSERT(!flag);

  // read another token; this should cause the writer to resume
  vftasks_read_int32(this->rport);

  // verify that writer has resumed
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testPassingLowWaterMark_write, arg)
{
  WPORT_FROM_VOID(wport, arg);

  // fill the channel
  for (int i = 0; i < 16; ++i) vftasks_write_int32(wport, i);

  // try to write another token; this should cause the writer to be suspended
  vftasks_write_int32(wport, 17);

  // exit writer thread
  return THREAD_EXIT_SUCCESS;
}

static void testPassingLowWaterMark_suspendWriter(vftasks_wport_t *wport)
{
  // exit writer thread
  THREAD_EXIT();
}

static void testPassingLowWaterMark_resumeWriter(vftasks_wport_t *wport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_wport(wport)) = true;
}

void StreamTest::testPassingLowWaterMark()
{
  bool flag;  // flag

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // set low-water mark
  vftasks_set_min_room(this->chan, 6);

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             testPassingLowWaterMark_suspendWriter,
                             testPassingLowWaterMark_resumeWriter,
                             suspendReader,
                             resumeReader);

  // initialize flag and add it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testPassingLowWaterMark_write, this->wport);

  // read five tokens
  for (int j = 0; j < 5; ++j) vftasks_read_int32(this->rport);

  // verify that writer hasn't resumed yet
  CPPUNIT_ASSERT(!flag);

  // read another token; this should normally cause the writer to resume
  vftasks_read_int32(this->rport);

  // but our writer won't resume anymore;
  // clear the flag and read another token
  flag = false;
  vftasks_read_int32(this->rport);

  // verify that the writer was again prompted to resume
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testHittingHighWaterMark_read, arg)
{
  RPORT_FROM_VOID(rport, arg);

  // try to read a token; this should cause the reader to be suspended
  vftasks_read_int32(rport);

  // exit writer thread
  return THREAD_EXIT_SUCCESS;
}

static void testHittingHighWaterMark_suspendReader(vftasks_rport_t *rport)
{
  // exit reader thread
  THREAD_EXIT();
}

static void testHittingHighWaterMark_resumeReader(vftasks_rport_t *rport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_rport(rport)) = true;
}

void StreamTest::testHittingHighWaterMark()
{
  bool flag;  // flag

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // set high-water mark
  vftasks_set_min_data(this->chan, 6);

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             suspendWriter,
                             resumeWriter,
                             testHittingHighWaterMark_suspendReader,
                             testHittingHighWaterMark_resumeReader);

  // initialize flag and add it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testHittingHighWaterMark_read, this->rport);

  // write five tokens
  for (int i = 0; i < 5; ++i) vftasks_write_int32(this->wport, i);

  // verify that reader hasn't resumed yet
  CPPUNIT_ASSERT(!flag);

  // write another token; this should cause the reader to resume
  vftasks_write_int32(this->wport, 6);

  // verify that reader has resumed
  CPPUNIT_ASSERT(flag);
}

static WORKER_PROTO(testPassingHighWaterMark_read, arg)
{
  RPORT_FROM_VOID(rport, arg);

  // try to read a token; this should cause the reader to be suspended
  vftasks_read_int32(rport);

  // exit reader thread
  return THREAD_EXIT_SUCCESS;
}

static void testPassingHighWaterMark_suspendReader(vftasks_rport_t *rport)
{
  // exit reader thread
  THREAD_EXIT();
}

static void testPassingHighWaterMark_resumeReader(vftasks_rport_t *rport)
{
  // set flag
  *(bool *)vftasks_get_chan_info(vftasks_chan_of_rport(rport)) = true;
}

void StreamTest::testPassingHighWaterMark()
{
  bool flag;  // flag

  CREATE_CHAN(16, 8) WITH_WPORT WITH_RPORT;

  // set high-water mark
  vftasks_set_min_data(this->chan, 6);

  // install channel hooks
  vftasks_install_chan_hooks(this->chan,
                             suspendWriter,
                             resumeWriter,
                             testPassingHighWaterMark_suspendReader,
                             testPassingHighWaterMark_resumeReader);

  // initialize flag and add it to the channel
  flag = false;
  vftasks_set_chan_info(this->chan, &flag);

  EXEC_ON_WORKER_THREAD(testPassingHighWaterMark_read, this->rport);

  // write five tokens
  for (int i = 0; i < 5; ++i) vftasks_write_int32(this->wport, i);

  // verify that reader hasn't resumed yet
  CPPUNIT_ASSERT(!flag);

  // write another token; this should normally cause the reader to resume
  vftasks_write_int32(this->wport, 6);

  // but our reader won't resume anymore;
  // clear the flag and write another token
  flag = false;
  vftasks_write_int32(this->wport, 7);

  // verify that the reader was again prompted to resume
  CPPUNIT_ASSERT(flag);
}

// register fixture
CPPUNIT_TEST_SUITE_REGISTRATION(StreamTest);
