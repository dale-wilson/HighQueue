// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <ComponentCommon/ComponentPch.h>
#define BOOST_TEST_NO_MAIN ComponentsTest
#include <boost/test/unit_test.hpp>

#include <Components/HeartbeatProducer.h>
#include <Components/TestMessageProducer.h>
#include <Components/TestMessageConsumer.h>
#include <Components/Arbitrator.h>
#include <Common/Stopwatch.h>
#include <Mocks/TestMessage.h>


using namespace HighQueue;
using namespace Components;
namespace
{
    struct GapMessage
    {
        uint32_t sequenceNumber_;

        GapMessage(uint32_t expectedSequenceNumber)
        : sequenceNumber_(expectedSequenceNumber)
        {
        }
    };

    const size_t testMessageExtras = 13;
    typedef TestMessage<testMessageExtras> ActualMessage;
    auto messageBytes = sizeof(ActualMessage);

    typedef TestMessageProducer<testMessageExtras> ProducerType;
    typedef std::shared_ptr<ProducerType> ProducerPtr;

    typedef TestMessageConsumer<testMessageExtras> ConsumerType;
    typedef std::shared_ptr<ConsumerType> ConsumerPtr;

    typedef Arbitrator<ActualMessage, GapMessage> ArbitratorType;
    typedef std::shared_ptr<ArbitratorType> ArbitratorPtr;
}

#define ENABLE_ARBITRATOR_TEST 01
#if ENABLE_ARBITRATOR_TEST
BOOST_AUTO_TEST_CASE(testArbitrator)
{
    size_t entryCount = 100000;
    size_t messageSize = sizeof(ActualMessage);
    uint32_t messageCount = 100000000;

    const size_t arbitratorLookAhead = 10000;

    const size_t numberOfHeartbeats = 1;  // Don't change this
    const size_t numberOfConsumers = 1;   // Don't change this
    const uint32_t numberOfProducers = 2;   // Don't change this
    const size_t numberOfArbitrators = 1; // Don't change this.

    const size_t queueCount = numberOfArbitrators + numberOfConsumers; // need a pool for each object that can receive messages
    // how many buffers do we need?
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = entryCount * queueCount + numberOfConsumers + numberOfHeartbeats + numberOfArbitrators + numberOfArbitrators * arbitratorLookAhead + numberOfProducers + extraMessages;

    const size_t spinCount = 0;
    const size_t yieldCount = 0;
    const size_t sleepCount = ConsumerWaitStrategy::FOREVER;
    const auto sleepTime = std::chrono::nanoseconds(10);
    ConsumerWaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepTime);

    CreationParameters parameters(strategy, entryCount, messageSize);
    MemoryPoolPtr memoryPool(new MemoryPool(messageSize, messagesNeeded));

    auto arbitratorConnection = std::make_shared<Connection>();
    arbitratorConnection->createLocal("Arbitrator", parameters, memoryPool);

    auto consumerConnection = std::make_shared<Connection>();
    consumerConnection->createLocal("Consumer", parameters, memoryPool);

    volatile bool producerGo = false;

    std::vector<ProducerPtr> producers;
    for(uint32_t nProducer = 0; nProducer < numberOfProducers; ++nProducer)
    {
        producers.emplace_back(new ProducerType(arbitratorConnection, producerGo, messageCount, nProducer, true));
    }
    auto asio = std::make_shared<AsioService>();
    auto arbitrator = std::make_shared<ArbitratorType>(arbitratorConnection, consumerConnection, arbitratorLookAhead, true);
    auto consumer = std::make_shared<ConsumerType>(consumerConnection, 0, true);
    std::chrono::milliseconds heartbeatInterval(10000);
    auto heartbeat = std::make_shared<HeartbeatProducer>(asio, arbitratorConnection, heartbeatInterval);
    // All wired up, ready to go.  Wait for the threads to initialize.
    arbitrator->start();
    for(auto producer : producers)
    {
        producer->start();
    }
    heartbeat->start();

    Stopwatch timer;
    producerGo = true;
    consumer->run();
    auto lapse = timer.nanoseconds();

    heartbeat->stop();
    for(auto producer : producers)
    {
        producer->stop();
    }
    arbitrator->stop();

    auto messageBits = messageBytes * 8;

    std::cerr << "Arbitration: ";
    std::cout << " Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(3) << double(messageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;
}
#endif // ENABLE_ARBITRATOR_TEST
