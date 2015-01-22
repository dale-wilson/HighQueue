// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>
#define BOOST_TEST_NO_MAIN StagesTest
#include <boost/test/unit_test.hpp>

#include <Stages/TestMessageProducer.h>
#include <Stages/TestMessageConsumer.h>

#include <Stages/QueueConsumer.h>
#include <Stages/QueueProducer.h>
#include <Stages/CopyPassThru.h>
#include <Stages/ForwardPassThru.h>
#include <Stages/BinaryPassThru.h>

#include <Common/Stopwatch.h>
#include <Common/Stopwatch.h>
#include <Mocks/TestMessage.h>


using namespace HighQueue;
using namespace Stages;
namespace
{
    const size_t testMessageExtras = 13;
    typedef TestMessage<testMessageExtras> ActualMessage;
    auto messageBytes = sizeof(ActualMessage);

    typedef TestMessageProducer<testMessageExtras> ProducerType;
    typedef std::shared_ptr<ProducerType> ProducerPtr;

    typedef TestMessageConsumer<testMessageExtras> ConsumerType;
    typedef std::shared_ptr<ConsumerType> ConsumerPtr;

#if 0
    typedef CopyPassThru<ActualMessage> CopierType;
#elif 0
    typedef BinaryPassThru CopierType;
#else
    typedef ForwardPassThru CopierType;
#endif
    typedef std::shared_ptr<CopierType> CopierPtr;
}

#define ENABLE_PIPELINE_TEST 01
#if ENABLE_PIPELINE_TEST
BOOST_AUTO_TEST_CASE(testPipeline)
{
    std::cout << "Queued Pipeline Test" << std::endl;
    size_t entryCount = 100000;
    size_t messageSize = sizeof(ActualMessage);
#if defined(_DEBUG)
    uint32_t messageCount = 10;
#else // _DEBUG
    uint32_t messageCount = 100000000;
#endif // _DEBUG

    const size_t numberOfConsumers = 1;   // Don't change this
    const size_t numberOfProducers = 1;   // Don't change this

    const size_t coreCount = std::thread::hardware_concurrency();
    const int32_t fudgeCopiers = 0; // if you want to see the clients fighting for cores, make this positive
    int32_t numberOfCopiers = int32_t(coreCount) - (numberOfConsumers + numberOfProducers) + fudgeCopiers;
    if(numberOfCopiers <= 0)
    {
        numberOfCopiers = 0;
        numberOfCopiers = 1; // for now, we always want at least one.
    }
 
    const size_t queueCount = numberOfCopiers + numberOfConsumers; // need a pool for each object that can receive messages
    // how many buffers do we need?
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = entryCount * queueCount + numberOfConsumers + numberOfCopiers + numberOfProducers + extraMessages;

    const size_t spinCount = 0;
    const size_t yieldCount = 0;
    const size_t sleepCount = WaitStrategy::FOREVER;
    const auto sleepTime = std::chrono::nanoseconds(10);
    WaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepTime);

    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messagesNeeded);
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
    
    volatile bool producerGo = false;

    std::vector <StagePtr> stages;
    
    auto producer = std::make_shared<ProducerType>(producerGo, messageCount, 1);
    producer->attachMemoryPool(memoryPool);
    stages.emplace_back(producer);

    auto producerPublisher = std::make_shared<QueueProducer>();
    producerPublisher->attachConnection(connections[0]);
    producer->attachDestination(producerPublisher);
    stages.emplace_back(producerPublisher);

    std::vector<CopierPtr> copiers;
    for(size_t nCopier = 1; nCopier < connections.size(); ++nCopier)
    {
        auto copyConsumer = std::make_shared<QueueConsumer>();
        copyConsumer->attachConnection(connections[nCopier-1]);
        stages.emplace_back(copyConsumer);

        auto copyPublisher = std::make_shared<QueueProducer>();
        copyPublisher->attachConnection(connections[nCopier]);
        copyConsumer->attachDestination(copyPublisher);
        stages.emplace_back(copyPublisher);
    }

    auto finalConsumer = std::make_shared<QueueConsumer>();
    finalConsumer->attachConnection(connections.back());
    stages.emplace_back(finalConsumer);

    auto consumer = std::make_shared<ConsumerType>(0);
    finalConsumer->attachDestination(consumer);

    // All wired up, ready to go.  Wait for the threads to initialize.
    for(auto & stage : stages)
    {
        stage->validate();
    }

    for(auto & stage : stages)
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

    std::cout << "Staged Pipeline " << (numberOfProducers + numberOfCopiers + numberOfConsumers) << " stage: ";
    std::cout << " Passed " << messageCount << ' ' << messageBytes << " byte messages in "
            << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  ";
    if(lapse == 0)
    {
        std::cout << "Run time too short to measure.   Use a larger messageCount" << std::endl;
    }
    else
    {
        std::cout
            << lapse / messageCount << " nsec./message "
            << std::setprecision(3) << double(messageCount * 1000) / double(lapse) << " MMsg/second "
#if defined(DISPLAY_PRONGHORN_STYLE_RESULTS)
            << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
            << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
#endif //DISPLAY_PRONGHORN_STYLE_RESULTS
            << std::endl;
    }
}
#endif // ENABLE_PIPELINE_TEST


#define ENABLE_DIRECT_PIPELINE_TEST 01
#if ENABLE_DIRECT_PIPELINE_TEST
BOOST_AUTO_TEST_CASE(testDirectPipeline)
{
    std::cout << "Direct Pipeline Test" << std::endl;
    size_t messageSize = sizeof(ActualMessage);
#if defined(_DEBUG)
    uint32_t messageCount = 10;
#else // _DEBUG
    uint32_t messageCount = 100000000;
#endif // _DEBUG

    const size_t numberOfConsumers = 1;   // Don't change this
    const size_t numberOfProducers = 1;   // Don't change this

    int32_t numberOfCopiers = 6; // pick a number, any number
 
    // how many buffers do we need?
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = numberOfCopiers + numberOfProducers + extraMessages;

    MemoryPoolPtr memoryPool(new MemoryPool(messageSize, messagesNeeded));

    volatile bool producerGo = false;
    auto producer = std::make_shared<ProducerType>(producerGo, messageCount, 1);
    producer->attachMemoryPool(memoryPool);

    StagePtr previous = producer;

    std::vector<CopierPtr> copiers;
    for(size_t nCopier = 0; nCopier < numberOfCopiers; ++nCopier)
    {
        copiers.emplace_back(new CopierType());
        auto & copier = copiers.back();
        copier->attachMemoryPool(memoryPool);
        previous->attachDestination(copier);
        previous = copier;
    }
    auto consumer = std::make_shared<ConsumerType>(0);
    previous->attachDestination(consumer);

    // All wired up.  See if everybody is happy
    producer->validate();
    for(auto & copier : copiers)
    {
        copier->validate();
    }
    consumer->validate();

    // turn everything on
    producer->start();
    for(auto & copier : copiers)
    {
        copier->start();
    }
    consumer->start();

    Stopwatch timer;
    producerGo = true;
    while(!consumer->isStopping())
    {
        std::this_thread::yield();
    }
    auto lapse = timer.nanoseconds();

    producer->stop();
    for(auto copier : copiers)
    {
        copier->stop();
    }

    producer->finish();
    for(auto & copier : copiers)
    {
        copier->finish();
    }
    consumer->finish();


    auto messageBits = messageBytes * 8;

    std::cout << "Direct Pipeline " << (numberOfProducers + numberOfCopiers + numberOfConsumers) << " stage: ";
    std::cout << " Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  ";
    if(lapse == 0)
    {
        std::cout << "Run time too short to measure.   Use a larger messageCount" << std::endl;
    }
    else
    {
        std::cout
            << lapse / messageCount << " nsec./message "
            << std::setprecision(3) << double(messageCount * 1000) / double(lapse) << " MMsg/second "
#if defined(DISPLAY_PRONGHORN_STYLE_RESULTS)
            << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
            << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
#endif //DISPLAY_PRONGHORN_STYLE_RESULTS
            << std::endl;
    }
}
#endif // ENABLE_DIRECT_PIPELINE_TEST
