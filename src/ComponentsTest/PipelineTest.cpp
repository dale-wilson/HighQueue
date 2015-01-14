// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <ComponentCommon/ComponentPch.h>
#define BOOST_TEST_NO_MAIN ComponentsTest
#include <boost/test/unit_test.hpp>

#include <Components/TestMessageProducer.h>
#include <Components/TestMessageConsumer.h>
#include <Components/PassThru.h>
#include <Common/Stopwatch.h>
#include <Common/Stopwatch.h>
#include <Mocks/TestMessage.h>


using namespace HighQueue;
using namespace Components;
namespace
{
    const size_t testMessageExtras = 13;
    typedef TestMessage<testMessageExtras> ActualMessage;
    auto messageBytes = sizeof(ActualMessage);

    typedef TestMessageProducer<testMessageExtras, NullHeaderGenerator> ProducerType;
    typedef std::shared_ptr<ProducerType> ProducerPtr;

    typedef TestMessageConsumer<testMessageExtras, NullHeaderGenerator> ConsumerType;
    typedef std::shared_ptr<ConsumerType> ConsumerPtr;

    typedef PassThru<ActualMessage> CopierType;
    typedef std::shared_ptr<CopierType> CopierPtr;
}

#define ENABLE_PIPELINE_TEST 1
#if ENABLE_PIPELINE_TEST
BOOST_AUTO_TEST_CASE(testPipeline)
{
    size_t entryCount = 100000;
    size_t messageSize = sizeof(ActualMessage);
    uint32_t messageCount = 100000000;

    const size_t numberOfConsumers = 1;   // Don't change this
    const size_t numberOfProducers = 1;   // Don't change this

    const size_t coreCount = std::thread::hardware_concurrency();
    const int32_t fudgeCopiers = -2; // if you want to see the clients fighting for cores, make this positive
    int32_t numberOfCopiers = int32_t(coreCount) - (numberOfConsumers + numberOfProducers) + fudgeCopiers;
    if(numberOfCopiers < 0)
    {
        numberOfCopiers = 0;
    }
 
    PassThru<ActualMessage>::CopyType copyType = PassThru<ActualMessage>::CopyBinary;

    const size_t queueCount = numberOfCopiers + numberOfConsumers; // need a pool for each object that can receive messages
    // how many buffers do we need?
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = entryCount * queueCount + numberOfConsumers + 2 * numberOfCopiers + numberOfProducers + extraMessages;

    const size_t spinCount = 0;
    const size_t yieldCount = 0;
    const size_t sleepCount = ConsumerWaitStrategy::FOREVER;
    const auto sleepTime = std::chrono::nanoseconds(10);
    ConsumerWaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepTime);

    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    MemoryPoolPtr memoryPool(new MemoryPool(messageSize, messagesNeeded));

    std::vector<ConnectionPtr> connections;
    for(size_t nConn = 0; nConn < queueCount; ++nConn)
    {
        std::shared_ptr<Connection> connection(new Connection);
        connections.push_back(connection);
        std::stringstream name;
        name << "Connection " << nConn;
        connection->createLocal(name.str(), parameters, memoryPool);
    }
    auto producer = std::make_shared<ProducerType>(connections[0], messageCount, 1, true);
    std::vector<CopierPtr> copiers;
    for(size_t nCopier = 1; nCopier < connections.size(); ++nCopier)
    {
        copiers.emplace_back(new CopierType(connections[nCopier - 1], connections[nCopier], copyType, 0, true));
    }

    auto consumer = std::make_shared<ConsumerType>(connections.back(), 0, true);

    // All wired up, ready to go.  Wait for the threads to initialize.
    volatile bool producerGo = false;
    producer->start(producerGo);
    for(auto copier : copiers)
    {
        copier->start();
    }

    Stopwatch timer;
    producerGo = true;
    consumer->run();
    auto lapse = timer.nanoseconds();

    producer->stop();
    for(auto copier : copiers)
    {
        copier->stop();
    }


    auto messageBits = messageBytes * 8;

    std::cerr << "Pipeline " << (numberOfProducers + numberOfCopiers + numberOfConsumers) << " stage. Copy type: " << copyType << ": ";
    std::cout << " Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(3) << double(messageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;
}
#endif // ENABLE_PIPELINE_TEST
