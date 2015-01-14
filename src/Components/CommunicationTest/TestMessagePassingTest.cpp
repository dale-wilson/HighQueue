// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Communication/CommunicationPch.h"
#define BOOST_TEST_NO_MAIN TestMessageProducer
#include <boost/test/unit_test.hpp>

#include <Communication/TestMessageProducer.h>
#include <Communication/TestMessageConsumer.h>
#include <Common/Stopwatch.h>

using namespace HighQueue;
using namespace Communication;

namespace
{
    const size_t testMessageExtras = 13;
    typedef TestMessage<testMessageExtras> ActualMessage;
    const size_t numberOfConsumers = 1; // do not change this
    const size_t maxNumberOfProducers = 1; // this you may change
}

#define ENABLE_TESTMESSAGEGENERATORTEST 1
#if ! ENABLE_TESTMESSAGEGENERATORTEST
#pragma message ("ENABLE_TESTMESSAGEGENERATORTEST " __FILE__)
#else // ENABLE_TESTMESSAGEGENERATORTEST

BOOST_AUTO_TEST_CASE(TestMessagePublisherTest)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 1000; 
    
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + numberOfConsumers + maxNumberOfProducers;
    uint32_t messageCount = 100000000;
    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    auto connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

    uint32_t producerNumber = 0;
    auto producer = std::make_shared<TestMessageProducer<testMessageExtras, NullHeaderGenerator> >(connection, messageCount, producerNumber);
    auto consumer = std::make_shared<TestMessageConsumer<testMessageExtras, NullHeaderGenerator> >(connection, messageCount, true);

    volatile bool startSignal = false;
    producer->start(startSignal);

    Stopwatch timer;
    startSignal = true;
    consumer->run();
    auto lapse = timer.nanoseconds();
    producer->stop();

    auto messageBytes = sizeof(ActualMessage);
    auto messageBits = sizeof(ActualMessage) * 8;
    std::cout << "Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(3) << double(messageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;


};

#endif // ENABLE_TESTMESSAGEGENERATORTEST