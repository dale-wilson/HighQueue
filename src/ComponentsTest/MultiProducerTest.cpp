// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <ComponentCommon/ComponentPch.h>
#define BOOST_TEST_NO_MAIN ComponentsTest
#include <boost/test/unit_test.hpp>

#include <Components/TestMessageProducer.h>
#include <Components/TestMessageConsumer.h>
#include <Common/Stopwatch.h>

using namespace HighQueue;
using namespace Components;

namespace
{
    const size_t testMessageExtras = 13;
    typedef TestMessage<testMessageExtras> ActualMessage;
    typedef TestMessageProducer<testMessageExtras> ProducerType;
    typedef std::shared_ptr<ProducerType> ProducerPtr;

    typedef TestMessageConsumer<testMessageExtras> ConsumerType;
    typedef std::shared_ptr<ConsumerType> ConsumerPtr;
}

#define ENABLE_MULTIPRODUCER_TEST 01
#if ! ENABLE_MULTIPRODUCER_TEST
#pragma message ("ENABLE_MULTIPRODUCER_TEST " __FILE__)
#else // ENABLE_MULTIPRODUCER_TEST

BOOST_AUTO_TEST_CASE(TestMultiProducers)
{
    size_t entryCount = 100000;
    
    size_t messageSize = sizeof(ActualMessage);
    uint32_t messageCount = 100000000;

    // Performance drops off severely when clients are competing for cores.
    // This is worth measauring, but not everytime.
    // You can see the beginning of the effect using this number because
    // the threads start competing with Windows itself for the last core.
    const size_t coreCount = std::thread::hardware_concurrency();
    const uint32_t extraProducers = 0; // if you want to see the clients fighting for cores, make this nonzero
    const uint32_t maxNumberOfProducers = uint32_t(coreCount == 1 ? coreCount : coreCount - 1) + extraProducers;

    const size_t numberOfConsumers = 1;  // Just for documentation, do not change this
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = entryCount + numberOfConsumers + maxNumberOfProducers + extraMessages;

    static const size_t spinCount = 0;
    static const size_t yieldCount = 0;
    static const size_t sleepCount = WaitStrategy::FOREVER;
    static const auto sleepTime = std::chrono::nanoseconds(10);
    WaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepTime);
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messagesNeeded);
    auto connection = std::make_shared<Connection>();
    connection->createLocal("LocalHQ", parameters);

    for(uint32_t producerCount = 1; producerCount <= maxNumberOfProducers; ++producerCount)
    {

        uint32_t perProducer = messageCount / producerCount;
        uint32_t perConsumer = perProducer * producerCount;

        volatile bool startSignal = false;
        std::vector<ProducerPtr> producers;
        for(uint32_t producerNumber = 0; producerNumber < producerCount; ++producerNumber)
        {
            producers.emplace_back(new ProducerType(connection, startSignal, perProducer, producerNumber, false));
        }

        auto consumer = std::make_shared<ConsumerType>(connection, perConsumer, true);

        for(auto producer : producers)
        { 
            producer->start();
        }
        std::this_thread::yield();

        Stopwatch timer;
        startSignal = true;
        consumer->run();
        auto lapse = timer.nanoseconds();
        for(auto producer : producers)
        {
            producer->stop();
        }

        auto messageBytes = sizeof(ActualMessage);
        auto messageBits = sizeof(ActualMessage) * 8;
        std::cout << "Test " << producerCount << " producer(s) ";
        std::cout << "passed " << perProducer << ' ' << messageBytes << " byte messages each in "
            << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
            << lapse / perConsumer << " nsec./message "
            << std::setprecision(3) << double(perConsumer) / double(lapse) << " GMsg/second "
            << std::setprecision(3) << double(perConsumer * messageBytes) / double(lapse) << " GByte/second "
            << std::setprecision(3) << double(perConsumer * messageBits) / double(lapse) << " GBit/second."
            << std::endl;

    }

};

#endif // ENABLE_MULTIPRODUCER_TEST