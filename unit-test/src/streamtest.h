#ifndef STREAMTEST_H
#define STREAMTEST_H

#include <cppunit/extensions/HelperMacros.h>

extern "C"
{
#include <vftasks.h>
}

class StreamTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StreamTest);

  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testCreatingTokenlessChannel);
  CPPUNIT_TEST(testCreatingChannelWithNegativeNumberOfTokens);
  CPPUNIT_TEST(testCreatingChannelWithZeroSizeTokens);
  CPPUNIT_TEST(testRoundingUpTokenSize);
  CPPUNIT_TEST(testConnectingMultipleWritePorts);
  CPPUNIT_TEST(testConnectingMultipleReadPorts);
  CPPUNIT_TEST(testWritePortRenewal);
  CPPUNIT_TEST(testReadPortRenewal);

  CPPUNIT_TEST(testInitialLowWaterMark);
  CPPUNIT_TEST(testSettingLowWaterMark);
  CPPUNIT_TEST(testMinimizingLowWaterMark);
  CPPUNIT_TEST(testSettingLowWaterMarkTooLow);
  CPPUNIT_TEST(testSettingLowWaterMarkWayTooLow);
  CPPUNIT_TEST(testMaximizingLowWaterMark);
  CPPUNIT_TEST(testSettingLowWaterMarkTooHigh);
  CPPUNIT_TEST(testSettingLowWaterMarkWayTooHigh);
  CPPUNIT_TEST(testInitialHighWaterMark);
  CPPUNIT_TEST(testSettingHighWaterMark);
  CPPUNIT_TEST(testMinimizingHighWaterMark);
  CPPUNIT_TEST(testSettingHighWaterMarkTooLow);
  CPPUNIT_TEST(testSettingHighWaterMarkWayTooLow);
  CPPUNIT_TEST(testMaximizingHighWaterMark);
  CPPUNIT_TEST(testSettingHighWaterMarkTooHigh);
  CPPUNIT_TEST(testSettingHighWaterMarkWayTooHigh);

  CPPUNIT_TEST(testInitialApplicationSpecificData);
  CPPUNIT_TEST(testSettingApplicationSpecificData);

  CPPUNIT_TEST(testRetrievingChannelFromWritePort);
  CPPUNIT_TEST(testRetrievingChannelFromReadPort);

  CPPUNIT_TEST(testInitialRoomAvailable);
  CPPUNIT_TEST(testRoomAvailableAfterWriting);
  CPPUNIT_TEST(testRoomAvailableAfterFilling);
  CPPUNIT_TEST(testRoomAvailableAfterWritingAndReading);
  CPPUNIT_TEST(testRoomAvailableAfterFillingAndReading);
  CPPUNIT_TEST(testRoomAvailableAfterWritingAndEmptying);
  CPPUNIT_TEST(testRoomAvailableAfterFillingAndEmptying);
  CPPUNIT_TEST(testInitialDataAvailable);
  CPPUNIT_TEST(testDataAvailableAfterWriting);
  CPPUNIT_TEST(testDataAvailableAfterFilling);
  CPPUNIT_TEST(testDataAvailableAfterWritingAndReading);
  CPPUNIT_TEST(testDataAvailableAfterFillingAndReading);
  CPPUNIT_TEST(testDataAvailableAfterWritingAndEmptying);
  CPPUNIT_TEST(testDataAvailableAfterFillingAndEmptying);

  CPPUNIT_TEST(testAcquiringRoomInitially);
  CPPUNIT_TEST(testAcquiringRoomAfterWriting);
  CPPUNIT_TEST(testAcquiringRoomAfterFilling);
  CPPUNIT_TEST(testAcquiringRoomAfterWritingAndReading);
  CPPUNIT_TEST(testAcquiringRoomAfterFillingAndReading);
  CPPUNIT_TEST(testAcquiringRoomAfterWritingAndEmptying);
  CPPUNIT_TEST(testAcquiringRoomAfterFillingAndEmptying);
  CPPUNIT_TEST(testAcquiringDataInitially);
  CPPUNIT_TEST(testAcquiringDataAfterWriting);
  CPPUNIT_TEST(testAcquiringDataAfterFilling);
  CPPUNIT_TEST(testAcquiringDataAfterWritingAndReading);
  CPPUNIT_TEST(testAcquiringDataAfterFillingAndReading);
  CPPUNIT_TEST(testAcquiringDataAfterWritingAndEmptying);
  CPPUNIT_TEST(testAcquiringDataAfterFillingAndEmptying);

  CPPUNIT_TEST(testSharedMemorySupport);
  CPPUNIT_TEST(testSharedMemoryMode);
  CPPUNIT_TEST(testFifoBehaviorInSharedMemoryMode);
  CPPUNIT_TEST(testTokenReuseInSharedMemoryMode);
  CPPUNIT_TEST(testWindowedMode);
  CPPUNIT_TEST(testFifoBehaviorInWindowedMode);
  CPPUNIT_TEST(testTokenReuseInWindowedMode);
  CPPUNIT_TEST(testKahnMode);
  CPPUNIT_TEST(testFifoBehaviorInKahnMode);
  CPPUNIT_TEST(testTokenReuseInKahnMode);

  CPPUNIT_TEST(testWrappingPutOffsets);
  CPPUNIT_TEST(testWrappingGetOffsets);
  CPPUNIT_TEST(testOverflow);

  CPPUNIT_TEST(testSuspendingWriter);
  CPPUNIT_TEST(testResumingWriter);
  CPPUNIT_TEST(testSuspendingReader);
  CPPUNIT_TEST(testResumingReader);

  CPPUNIT_TEST(testHittingLowWaterMark);
  CPPUNIT_TEST(testPassingLowWaterMark);
  CPPUNIT_TEST(testHittingHighWaterMark);
  CPPUNIT_TEST(testPassingHighWaterMark);

  CPPUNIT_TEST_SUITE_END();  // StreamTest

public:
  void increaseWriterSuspendCount();
  int getWriterSuspendCount();
  void increaseWriterResumeCount();
  int getWriterResumeCount();
  void increaseReaderSuspendCount();
  int getReaderSuspendCount();
  void increaseReaderResumeCount();
  int getReaderResumeCount();

  void setUp();
  void tearDown();

  void testCreation();
  void testCreatingTokenlessChannel();
  void testCreatingChannelWithNegativeNumberOfTokens();
  void testCreatingChannelWithZeroSizeTokens();
  void testRoundingUpTokenSize();
  void testConnectingMultipleWritePorts();
  void testConnectingMultipleReadPorts();
  void testWritePortRenewal();
  void testReadPortRenewal();

  void testInitialLowWaterMark();
  void testSettingLowWaterMark();
  void testMinimizingLowWaterMark();
  void testSettingLowWaterMarkTooLow();
  void testSettingLowWaterMarkWayTooLow();
  void testMaximizingLowWaterMark();
  void testSettingLowWaterMarkTooHigh();
  void testSettingLowWaterMarkWayTooHigh();
  void testInitialHighWaterMark();
  void testSettingHighWaterMark();
  void testMinimizingHighWaterMark();
  void testSettingHighWaterMarkTooLow();
  void testSettingHighWaterMarkWayTooLow();
  void testMaximizingHighWaterMark();
  void testSettingHighWaterMarkTooHigh();
  void testSettingHighWaterMarkWayTooHigh();

  void testInitialApplicationSpecificData();
  void testSettingApplicationSpecificData();

  void testRetrievingChannelFromWritePort();
  void testRetrievingChannelFromReadPort();

  void testInitialRoomAvailable();
  void testRoomAvailableAfterWriting();
  void testRoomAvailableAfterFilling();
  void testRoomAvailableAfterWritingAndReading();
  void testRoomAvailableAfterFillingAndReading();
  void testRoomAvailableAfterWritingAndEmptying();
  void testRoomAvailableAfterFillingAndEmptying();
  void testInitialDataAvailable();
  void testDataAvailableAfterWriting();
  void testDataAvailableAfterFilling();
  void testDataAvailableAfterWritingAndReading();
  void testDataAvailableAfterFillingAndReading();
  void testDataAvailableAfterWritingAndEmptying();
  void testDataAvailableAfterFillingAndEmptying();

  void testAcquiringRoomInitially();
  void testAcquiringRoomAfterWriting();
  void testAcquiringRoomAfterFilling();
  void testAcquiringRoomAfterWritingAndReading();
  void testAcquiringRoomAfterFillingAndReading();
  void testAcquiringRoomAfterWritingAndEmptying();
  void testAcquiringRoomAfterFillingAndEmptying();
  void testAcquiringDataInitially();
  void testAcquiringDataAfterWriting();
  void testAcquiringDataAfterFilling();
  void testAcquiringDataAfterWritingAndReading();
  void testAcquiringDataAfterFillingAndReading();
  void testAcquiringDataAfterWritingAndEmptying();
  void testAcquiringDataAfterFillingAndEmptying();

  void testSharedMemorySupport();
  void testSharedMemoryMode();
  void testFifoBehaviorInSharedMemoryMode();
  void testTokenReuseInSharedMemoryMode();
  void testWindowedMode();
  void testFifoBehaviorInWindowedMode();
  void testTokenReuseInWindowedMode();
  void testKahnMode();
  void testFifoBehaviorInKahnMode();
  void testTokenReuseInKahnMode();

  void testWrappingPutOffsets();
  void testWrappingGetOffsets();
  void testOverflow();

  void testSuspendingWriter();
  void testResumingWriter();
  void testSuspendingReader();
  void testResumingReader();

  void testHittingLowWaterMark();
  void testPassingLowWaterMark();
  void testHittingHighWaterMark();
  void testPassingHighWaterMark();

private:
  vftasks_malloc_t *mem_mgr;  // pointer to a memory manager
  vftasks_chan_t *chan;       // pointer to a channel
  vftasks_wport_t *wport;     // pointer to a write port
  vftasks_rport_t *rport;     // pointer to a read port
  int writerSuspendCount;     // writer-suspend count
  int writerResumeCount;      // writer-resume count
  int readerSuspendCount;     // reader-suspend count
  int readerResumeCount;      // reader-resume count
};

#endif  // STREAMTEST_H
