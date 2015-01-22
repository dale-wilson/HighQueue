// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>
#define BOOST_TEST_NO_MAIN StagesTest
#include <boost/test/unit_test.hpp>

#include <Stages/OrderedMerge.h>
#include <Stages/HeartbeatProducer.h>
#include <Stages/TestMessageProducer.h>
#include <Stages/TestMessageConsumer.h>
#include <Stages/QueueConsumer.h>
#include <Stages/QueueProducer.h>

#include <Common/ReverseRange.h>

#include <Common/Stopwatch.h>

using namespace HighQueue;
using namespace Stages;
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

}

#define ENABLE_ORDEREDMERGE_TEST 01
#if ENABLE_ORDEREDMERGE_TEST
BOOST_AUTO_TEST_CASE(testOrderedMerge)
{
    std::cout << "OrderedMerge" << std::endl << std::flush;
    size_t entryCount = 10000;
    size_t messageSize = sizeof(ActualMessage);
#if defined (_DEBUG)
	uint32_t messageCount = 10;
#else // _DEBUG
    uint32_t messageCount = 10000000;
#endif // _DEBUG
    const size_t lookAhead = 1000; // real world numbers would be in the thousands.

    const size_t numberOfHeartbeats = 1;  // Don't change this
    const size_t numberOfConsumers = 1;   // Don't change this
    const uint32_t numberOfProducers = 2;   // Don't change this
    const size_t numberOforderedMerges = 1; // Don't change this.

    const size_t queueCount = numberOforderedMerges + numberOfConsumers; // need a pool for each object that can receive messages
    // how many buffers do we need?
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = entryCount * queueCount + numberOfConsumers + numberOfHeartbeats + numberOforderedMerges * 2 + numberOforderedMerges * lookAhead + numberOfProducers + extraMessages;

    auto asio = std::make_shared<AsioService>();
    std::chrono::milliseconds heartbeatInterval(10000);

    const size_t spinCount = 0;
    const size_t yieldCount = 0;
    const size_t sleepCount = WaitStrategy::FOREVER;
    const auto sleepTime = std::chrono::nanoseconds(10);
    WaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepTime);

    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize);
    MemoryPoolPtr memoryPool(new MemoryPool(messageSize, messagesNeeded));

    auto mergeConnection = std::make_shared<Connection>();
    mergeConnection->createLocal("OrderedMerge", parameters, memoryPool);

    volatile bool producerGo = false;

    typedef std::vector<StagePtr> StagesVec;
    StagesVec stages;


    for(uint32_t nProducer = 0; nProducer < numberOfProducers; ++nProducer)
    {
        auto producer = std::make_shared<ProducerType>(producerGo, messageCount, nProducer);
        stages.emplace_back(producer);
        producer->attachMemoryPool(memoryPool);

        auto publisher = std::make_shared<QueueProducer>();
        stages.emplace_back(publisher);
        publisher->attachConnection(mergeConnection);
        producer->attachDestination(publisher);

    }
    auto heartbeat = std::make_shared<HeartbeatProducer>(heartbeatInterval);
    stages.emplace_back(heartbeat);
    heartbeat->attachMemoryPool(memoryPool);
    heartbeat->attachIoService(asio);

    auto heartbeatPublisher = std::make_shared<QueueProducer>();
    stages.emplace_back(heartbeatPublisher);
    heartbeatPublisher->attachConnection(mergeConnection);
    heartbeat->attachDestination(heartbeatPublisher);

    auto queueConsumer = std::make_shared<QueueConsumer>();
    stages.emplace_back(queueConsumer);
    queueConsumer->attachConnection(mergeConnection);

    auto orderedMerge = std::make_shared<OrderedMerge>(lookAhead);
    stages.emplace_back(orderedMerge);
    orderedMerge->attachMemoryPool(memoryPool);
    queueConsumer->attachDestination(orderedMerge);

    auto consumer = std::make_shared<ConsumerType>(messageCount);
    orderedMerge->attachDestination(consumer);

    // All wired up, ready to go.
    ReverseRange<StagesVec> rstages(stages);
    for(auto stage : rstages)
    {
        stage->validate();
    }
 
    for(auto stage : rstages)
    {
        stage->start();
    }

    Stopwatch timer;
    producerGo = true;
    while(!consumer->isStopping())
    {
        std::this_thread::yield();
    }
    auto lapse = timer.nanoseconds();

    for(auto stage : stages)
    {
        stage->stop();
    }

    for(auto stage : stages)
    {
        stage->finish();
    }

    auto messageBits = messageBytes * 8;

    std::cout << "OrderedMerge: " << std::fixed;
    std::cout << " Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message ";
    if(lapse == 0)
    {
        std::cout << " Lapse is too short to measure.  Increase messageCount." << std::endl;
    }
    else
    {
        std::cout
            << std::setprecision(3) << double(messageCount) * 1000.0L / double(lapse) << " MMsg/second "
#ifdef DISPLAY_PRONGHORN_STYLE_RESULTS
            << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
            << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
#endif // DISPLAY_PRONGHORN_STYLE_RESULTS
            << std::endl;
    }
}
#endif // ENABLE_ORDEREDMERGE_TEST
