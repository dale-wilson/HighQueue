#include "Common/HSQueuePch.h"
#define BOOST_TEST_NO_MAIN HSQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HSQueue/Producer.h>
#include <HSQueue/Consumer.h>
#include <Common/Stopwatch.h>
#include <HSQPerformance/TestMessage.h>

using namespace HSQueue;
typedef TestMessage<10> ActualMessage;

namespace
{
    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;

    void producerFunction(Connection & connection, uint32_t producerNumber, uint64_t messageCount, bool solo)
    {
        Producer producer(connection, solo);
        HSQueue::Message producerMessage;
        if(!connection.allocate(producerMessage))
        {
            std::cerr << "Failed to allocate message for producer Number " << producerNumber << std::endl;
            return;
        }

        ++threadsReady;
        while(!producerGo)
        {
            std::this_thread::yield();
        }

        for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
        {
            auto testMessage = producerMessage.construct<ActualMessage>(producerNumber, messageNumber);
            producer.publish(producerMessage);
        }
    }
}

#define DISABLE_MultithreadMessagePassingPerformancex
#ifdef DISABLE_MultithreadMessagePassingPerformance
#pragma message ("DISABLE_MultithreadMessagePassingPerformance")
#else // DISABLE_MultithreadMessagePassingPerformance 
BOOST_AUTO_TEST_CASE(testMultithreadMessagePassingPerformance)
{
    static const size_t entryCount = 100000;
    static const size_t messageSize = sizeof(ActualMessage);

    static const uint64_t targetMessageCount = 1000000 * 100; // runs about 5 to 10 seconds in release/optimized build
    static const size_t producerLimit = 2;//10; // running on 8 core system.  Once we go over 7 producers it slows down.  That's one thing we want to see.
    static const size_t consumerLimit = 1;  // Just for documentation
    static const size_t messageCount = entryCount + consumerLimit +  producerLimit;

    static const size_t spinCount = 0;
    static const size_t yieldCount = ConsumerWaitStrategy::FOREVER;

    ConsumerWaitStrategy strategy(spinCount, yieldCount);
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    Consumer consumer(connection);
    HSQueue::Message consumerMessage;
    BOOST_REQUIRE(connection.allocate(consumerMessage));

    for(size_t producerCount = 1; producerCount < producerLimit; ++producerCount)
    {
        std::cerr << "Test " << producerCount << " producer";

        std::vector<std::thread> producerThreads;
        std::vector<uint64_t> nextMessage;

        threadsReady = 0;
        producerGo = false;
        size_t perProducer = targetMessageCount / producerCount;
        size_t actualMessageCount = perProducer * producerCount;

        for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
        {
            nextMessage.emplace_back(0u);
            producerThreads.emplace_back(
                std::bind(producerFunction, std::ref(connection), nTh, perProducer, producerCount == 1));
        }
        std::this_thread::yield();

        while(threadsReady < producerCount)
        {
            std::this_thread::yield();
        }

        Stopwatch timer;
        producerGo = true;

        for(uint64_t messageNumber = 0; messageNumber < actualMessageCount; ++messageNumber)
        {
            consumer.getNext(consumerMessage);
            auto testMessage = consumerMessage.get<ActualMessage>();
            testMessage->touch();
            auto & msgNumber = nextMessage[testMessage->producerNumber()];
            if(msgNumber != testMessage->messageNumber())
            {
                // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
                BOOST_CHECK_EQUAL(messageNumber, testMessage->messageNumber());
            }
            ++ msgNumber; 
        }

        auto lapse = timer.nanoseconds();

        // sometimes we synchronize thread shut down.
        producerGo = false;

        for(size_t nTh = 0; nTh < producerCount; ++nTh)
        {
            producerThreads[nTh].join();
        }

        auto messageBytes = sizeof(ActualMessage);
        auto messageBits = sizeof(ActualMessage) * 8;
        std::cout << " Passed " << actualMessageCount << ' ' << messageBytes << " byte messages in "
            << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  " 
            << lapse / actualMessageCount << " nsec./message "
            << std::setprecision(3) << double(actualMessageCount) / double(lapse) << " GMsg/second "
            << std::setprecision(3) << double(actualMessageCount * messageBytes) / double(lapse) << " GByte/second "
            << std::setprecision(3) << double(actualMessageCount * messageBits) / double(lapse) << " GBit/second."
            << std::endl;
    }
}
#endif // DISABLE_MultithreadMessagePassingPerformance
