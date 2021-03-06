// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>
#define BOOST_TEST_NO_MAIN StepsPerformance
#include <boost/test/unit_test.hpp>

#include <StepLibrary/MockMessageProducer.hpp>
#include <StepLibrary/MockMessageConsumer.hpp>
#include <StepLibrary/InputQueue.hpp>
#include <StepLibrary/SendToQueue.hpp>
#include <StepLibrary/Tee.hpp>

#include <HighQueue/Consumer.hpp>
#include <HighQueue/Producer.hpp>

#include <Common/Stopwatch.hpp>
#include <Common/ReverseRange.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    typedef SmallMockMessage ActualMessage;
    typedef MockMessageProducer<ActualMessage> ProducerType;
    typedef std::shared_ptr<ProducerType> ProducerPtr;

    typedef MockMessageConsumer<ActualMessage> ConsumerType;
    typedef std::shared_ptr<ConsumerType> ConsumerPtr;
}

#define ENABLE_MULTIPRODUCER_TEST 0
#if ! ENABLE_MULTIPRODUCER_TEST
#pragma message ("ENABLE_MULTIPRODUCER_TEST " __FILE__)
#else // ENABLE_MULTIPRODUCER_TEST

BOOST_AUTO_TEST_CASE(TestMultiProducers)
{
    std::cout << "Multiproducer test" << std::endl;
    size_t entryCount = 10000;
    
    size_t messageSize = sizeof(ActualMessage);
#if defined(_DEBUG)
    uint32_t messageCount = 10;
#else // _DEBUG
    uint32_t messageCount = 100000000;
#endif // _DEBUG

    // Performance drops off severely when clients are competing for cores.
    // This is worth measauring, but not everytime.
    // You can see the beginning of the effect using this number because
    // the threads start competing with Windows itself for the last core.
    const size_t coreCount = std::thread::hardware_concurrency();
    const int32_t extraProducers = 0; // if you want to see the clients fighting for cores, make this positive
    const uint32_t maxNumberOfProducers = uint32_t(coreCount <= 1 ? 1 : coreCount - 1) + extraProducers;

    const size_t numberOfConsumers = 1;  // Just for documentation, do not change this
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = entryCount + numberOfConsumers + maxNumberOfProducers + extraMessages;

    MemoryPoolPtr memoryPool = std::make_shared<MemoryPool>(messageSize, messagesNeeded);

    static const size_t spinCount = 0;
    static const size_t yieldCount = 0;
    static const size_t sleepCount = WaitStrategy::FOREVER;
    static const auto sleepTime = std::chrono::nanoseconds(10);
    WaitStrategy strategy(spinCount, yieldCount, sleepCount, sleepTime);
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, 0);
    auto connection = std::make_shared<Connection>();
    connection->createLocal("LocalHQ", parameters, memoryPool);

    for(uint32_t producerCount = 1; producerCount <= maxNumberOfProducers; ++producerCount)
    {
        uint32_t perProducer = messageCount / producerCount;
        uint32_t perConsumer = perProducer * producerCount;

        volatile bool startSignal = false;
        typedef std::vector<StepPtr> Steps;
        Steps steps;
        for(uint32_t producerNumber = 0; producerNumber < producerCount; ++producerNumber)
        {
            auto producer = std::make_shared<ProducerType>(&startSignal, perProducer, producerNumber);
            steps.emplace_back(producer);
            producer->attachMemoryPool(memoryPool);

            auto queueProducer = std::make_shared<SendToQueue>();
            steps.emplace_back(queueProducer);
            queueProducer->configureSolo(producerCount == 1);
            producer->attachDestination(queueProducer);
            queueProducer->attachConnection(connection);
        }

        auto queueConsumer = std::make_shared<InputQueue>();
        steps.emplace_back(queueConsumer);
        queueConsumer->attachConnection(connection);
        auto consumer = std::make_shared<ConsumerType>(perConsumer);
        steps.emplace_back(consumer);

#define USE_T 0
#if USE_T
        auto tee = std::make_shared<Tee>();
        steps.emplace_back(tee);
        tee->attachOutputStream(& std::cerr);
        queueConsumer->attachDestination(tee);
        tee->attachDestination(consumer);
#else // USE_T
        queueConsumer->attachDestination(consumer);
#endif // USE_T
        for(auto & step : steps)
        { 
            step->validate();
        }

        for (auto & step : ReverseRange<Steps>(steps))
        {
            step->start();
        }

        ////////////////////////
        // Begin the actual test.
        Stopwatch timer;
        startSignal = true;

        while(!consumer->isStopping())
        {
            std::this_thread::yield();
        }

        auto lapse = timer.nanoseconds();
        // End the test
        ///////////////

        for(auto & step : steps)
        {
            step->stop();
        }

        for(auto & step : steps)
        {
            step->finish();
        }

        auto messageBytes = sizeof(ActualMessage);
        std::cout << "Stepd Test " << producerCount << " producer(s) ";
        std::cout << "passed " << perProducer << ' ' << messageBytes << " byte messages each in "
            << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  ";
        if(lapse == 0)
        {
            std::cout << "Run time too short to measure.   Use a larger messageCount" << std::endl;
        }
        else
        {
            std::cout
                << lapse / perConsumer << " nsec./message "
                << std::setprecision(3) << double(perConsumer) * 1000.0L / double(lapse) << " MMsg/second "
                << std::endl;
        }
    }

};

#endif // ENABLE_MULTIPRODUCER_TEST