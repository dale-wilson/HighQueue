// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Communication/CommunicationPch.h"
#define BOOST_TEST_NO_MAIN TestMessagePublisher
#include <boost/test/unit_test.hpp>

#include <Communication/TestMessagePublisher.h>
#include <Communication/AsioService.h>
#include <HighQueue/Connection.h>
#include <Mocks/TestMessage.h>

using namespace HighQueue;
using namespace Communication;

namespace
{
    const size_t testMessageExtras = 20;
    typedef TestMessage<testMessageExtras> ActualMessage;
}

#define ENABLE_TESTMESSAGEGENERATORTEST 1
#if ! ENABLE_TESTMESSAGEGENERATORTEST
#pragma message ("ENABLE_TESTMESSAGEGENERATORTEST " __FILE__)
#else // ENABLE_TESTMESSAGEGENERATORTEST

BOOST_AUTO_TEST_CASE(TestMessagePublisherTest)
{
    auto connection = std::make_shared<Connection>();
    ConsumerWaitStrategy strategy;
    size_t entryCount = 262144 / 2; 
    
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    uint32_t messageCount = 1000000 * 100;
    uint32_t producerNumber = 0;
    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    connection->createLocal("LocalIv", parameters);

    auto publisher = std::make_shared<TestMessagePublisher<testMessageExtras, NullHeaderGenerator> >(connection, messageCount, producerNumber);

};

#endif // ENABLE_TESTMESSAGEGENERATORTEST