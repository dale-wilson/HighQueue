// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>
#define BOOST_TEST_NO_MAIN StepsPerformance
#include <boost/test/unit_test.hpp>

#include <StepLibrary/OrderedMerge.hpp>
#include <StepLibrary/Shuffler.hpp>
#include <StepLibrary/HeartbeatProducer.hpp>
#include <StepLibrary/MockMessageProducer.hpp>
#include <StepLibrary/MockMessageConsumer.hpp>
#include <StepLibrary/InputQueue.hpp>
#include <StepLibrary/SendToQueue.hpp>

#include <Common/ReverseRange.hpp>

#include <Common/Stopwatch.hpp>

using namespace HighQueue;
using namespace Steps;
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

    typedef MediumMockMessage ActualMessage;
    auto messageBytes = sizeof(ActualMessage);

    typedef MockMessageProducer<ActualMessage> ProducerType;
    typedef std::shared_ptr<ProducerType> ProducerPtr;

    typedef MockMessageConsumer<ActualMessage> ConsumerType;
    typedef std::shared_ptr<ConsumerType> ConsumerPtr;

}

#define ENABLE_ORDEREDMERGE_TEST 0
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
    const size_t lookAhead = 50; // real world numbers would be in the thousands.
    const size_t shuffleAhead = lookAhead + 10;

    const uint32_t numberOfHeartbeats = 1;  // Don't change this
    const uint32_t numberOfConsumers = 1;   // Don't change this
    const uint32_t numberOfProducers = 2;   // Don't change this
    const uint32_t numberOforderedMerges = 1; // Don't change this.
    const uint32_t numberOfShufflers = numberOfProducers + 1 / 2;
    const size_t queueCount = numberOforderedMerges + numberOfConsumers; // need a pool for each object that can receive messages
    // how many buffers do we need?
    size_t extraMessages = 0; // in case we need it someday (YAGNI)
    const size_t messagesNeeded = 
        entryCount * queueCount + 
        numberOfConsumers + 
        numberOfHeartbeats + 
        numberOforderedMerges + numberOforderedMerges * lookAhead + 
        numberOfShufflers + numberOfShufflers * shuffleAhead +
        numberOfProducers + 
        extraMessages;

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

    typedef std::vector<StepPtr> StepsVec;
    StepsVec steps;


    for(uint32_t nProducer = 0; nProducer < numberOfProducers; ++nProducer)
    {
        auto producer = std::make_shared<ProducerType>(&producerGo, messageCount, nProducer);
        steps.emplace_back(producer);
        producer->attachMemoryPool(memoryPool);

        auto publisher = std::make_shared<SendToQueue>();
        steps.emplace_back(publisher);
        publisher->attachConnection(mergeConnection);

        if (nProducer % 2 == 0)
        {
#ifdef USE_SHUFFLER
            auto shuffler = std::make_shared<Shuffler>(shuffleAhead);
            steps.emplace_back(shuffler);
            shuffler->attachMemoryPool(memoryPool);

            producer->attachDestination(shuffler);
            shuffler->attachDestination(publisher);
#else // ShUFFLER
            producer->attachDestination(publisher);
#endif // SHUFFLER
        }
        else
        {
            producer->attachDestination(publisher);
        }
    }
    auto heartbeat = std::make_shared<HeartbeatProducer>(heartbeatInterval);
    steps.emplace_back(heartbeat);
    heartbeat->attachMemoryPool(memoryPool);
    heartbeat->attachIoService(asio);

    auto heartbeatPublisher = std::make_shared<SendToQueue>();
    steps.emplace_back(heartbeatPublisher);
    heartbeatPublisher->attachConnection(mergeConnection);
    heartbeat->attachDestination(heartbeatPublisher);

    auto queueConsumer = std::make_shared<InputQueue>();
    steps.emplace_back(queueConsumer);
    queueConsumer->attachConnection(mergeConnection);

    auto orderedMerge = std::make_shared<OrderedMerge>(lookAhead);
    steps.emplace_back(orderedMerge);
    orderedMerge->attachMemoryPool(memoryPool);
    queueConsumer->attachDestination(orderedMerge);

    auto consumer = std::make_shared<ConsumerType>(messageCount);
    orderedMerge->attachDestination(consumer);

    // All wired up, ready to go.
    ReverseRange<StepsVec> rsteps(steps);
    for(auto & step : rsteps)
    {
        step->validate();
    }
 
    for(auto & step : rsteps)
    {
        step->start();
    }

    asio->runThreads(1, false);

    //////////////////
    // start the test
    Stopwatch timer;
    producerGo = true;
    while(!consumer->isStopping())
    {
        std::this_thread::yield();
    }
    auto lapse = timer.nanoseconds();
    // the test ends here
    /////////////////////
    for(auto & step : steps)
    {
        step->stop();
    }

    asio->stopService();
    asio->joinThreads();

    for(auto & step : steps)
    {
        step->finish();
    }

    BOOST_CHECK_EQUAL(consumer->messagesHandled(), messageCount);
    BOOST_CHECK_EQUAL(consumer->errors(), 0U); // this test will not be valid if we introduce gaps.

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
    std::cout << "Ordered Merge statistics: " << std::endl;
    orderedMerge->writeStats(std::cout);
    std::cout << std::endl << std::endl;
}
#endif // ENABLE_ORDEREDMERGE_TEST
