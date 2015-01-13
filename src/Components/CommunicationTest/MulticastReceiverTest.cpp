// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Communication/CommunicationPch.h"
#define BOOST_TEST_NO_MAIN MulticastReceiver
#include <boost/test/unit_test.hpp>

#include <Communication/MulticastReceiver.h>
#include <Communication/AsioService.h>
#include <HighQueue/Connection.h>
#include <HQPerformance/TestMessage.h>

using namespace HighQueue;
using namespace Communication;
typedef TestMessage</*13*/80> ActualMessage;

namespace
{
}

#define ENABLE_MULTICASTRECEIVERTEST 1
#if ! ENABLE_MULTICASTRECEIVERTEST
#pragma message ("ENABLE_MULTICASTRECEIVERTEST " __FILE__)
#else // ENABLE_MULTICASTRECEIVERTEST

BOOST_AUTO_TEST_CASE(MulticastReceiverTest)
{
    auto ioservice = std::make_shared<AsioService>();
    auto connection = std::make_shared<Connection>();
    ConsumerWaitStrategy strategy;
    size_t entryCount = 262144 / 2; // <- thats the number of messages in the primaryRingBuffer in the pronghorn test //100000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    uint64_t messageCount = 1000000 * 100;
    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    connection->createLocal("LocalIv", parameters);

    MulticastConfiguration configuration("multicastGroupIP", "listenInterfaceIP", "bindIP", 10000);
    auto receiver = std::make_shared<MulticastReceiver<NullHeaderGenerator> >(ioservice, connection);
    BOOST_CHECK(receiver->configure(configuration));
};

#endif // ENABLE_MULTICASTRECEIVERTEST