// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>
#define BOOST_TEST_NO_MAIN StepsPerformance
#include <boost/test/unit_test.hpp>

#include <StepLibrary/MulticastReceiver.hpp>
#include <Steps/AsioService.hpp>
#include <HighQueue/Connection.hpp>
#include <Mocks/MockMessage.hpp>

using namespace HighQueue;
using namespace Steps;
typedef MockMessage</*13*/80> ActualMessage;

namespace
{
}

#define ENABLE_MULTICASTRECEIVERTEST 0
#if ! ENABLE_MULTICASTRECEIVERTEST
#pragma message ("ENABLE_MULTICASTRECEIVERTEST " __FILE__)
#else // ENABLE_MULTICASTRECEIVERTEST
BOOST_AUTO_TEST_CASE(MulticastReceiverTest)
{
    auto ioservice = std::make_shared<AsioService>();
    auto connection = std::make_shared<Connection>();
    WaitStrategy strategy;
    size_t entryCount = 262144 / 2; // <- thats the number of messages in the primaryRingBuffer in the pronghorn test //100000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
#if defined(_DEBUG)
    uint32_t messageCount = 10;
#else // _DEBUG
    uint32_t messageCount = 100000000;
#endif // _DEBUG
    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    connection->createLocal("LocalIv", parameters);

    MulticastConfiguration configuration("multicastGroupIP", "listenInterfaceIP", "bindIP", 10000);
    auto receiver = std::make_shared<MulticastReceiver>(ioservice, connection);
    BOOST_CHECK(receiver->configure(configuration));
};

#endif // ENABLE_MULTICASTRECEIVERTEST