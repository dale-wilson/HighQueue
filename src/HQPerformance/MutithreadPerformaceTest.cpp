#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>
#include <HighQueue/Consumer.h>
#include <Common/Stopwatch.h>
#include <HQPerformance/TestMessage.h>

using namespace HighQueue;
typedef TestMessage<13> ActualMessage;

namespace
{
    volatile std::atomic<uint32_t> threadsReady;
    volatile bool producerGo = false;

    void producerFunction(Connection & connection, uint32_t producerNumber, uint64_t messageCount, bool solo)
    {
        try
        {
            Producer producer(connection, solo);
            Message producerMessage(connection);
        
            ++threadsReady;
            while(!producerGo)
            {
                std::this_thread::yield();
            }

            for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
            {
                auto testMessage = producerMessage.emplace<ActualMessage>(producerNumber, messageNumber);
                producer.publish(producerMessage);
            }
        }
        catch(const std::exception & ex)
        {
            std::cerr << "Producer Number " << producerNumber << " failed. " << ex.what() << std::endl;
            return;
        }
    }
}


#define DISABLE_ST_PERFORMANCEx
#ifndef DISABLE_ST_PERFORMANCE
BOOST_AUTO_TEST_CASE(testSingleThreadedMessagePassingPerformance)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 262144 / 2; // <- thats the number of messages in the primaryRingBuffer in the pronghorn test //100000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    uint64_t messageCount = 1000000 * 100;

    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    Connection connection;
    connection.createLocal("LocalIv", parameters);


    Producer producer(connection);
    Consumer consumer(connection);
    Message producerMessage(connection);
    Message consumerMessage(connection);

    std::cout << "Single threaded: ";

    Stopwatch timer;

    for(uint32_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
    {
        producerMessage.emplace<ActualMessage>(1, messageNumber);
        producer.publish(producerMessage);
        consumer.getNext(consumerMessage);
        auto testMessage = consumerMessage.get<ActualMessage>();
        if(messageNumber != testMessage->messageNumber())
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->messageNumber());
        }
    }
    auto lapse = timer.nanoseconds();
    auto messageBytes = sizeof(ActualMessage);
    auto messageBits = sizeof(ActualMessage) * 8;
    std::cout << "Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(3) << double(messageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;
}
#endif // DISABLE_ST_PERFORMANCE


#define DISABLE_MultithreadMessagePassingPerformancex
#ifdef DISABLE_MultithreadMessagePassingPerformance
#pragma message ("DISABLE_MultithreadMessagePassingPerformance")
#else // DISABLE_MultithreadMessagePassingPerformance 
BOOST_AUTO_TEST_CASE(testMultithreadMessagePassingPerformance)
{
    static const size_t entryCount = 100000;
    static const size_t messageSize = sizeof(ActualMessage);

    static const uint64_t targetMessageCount = 1000000 * 100; // runs about 5 to 10 seconds in release/optimized build
    static const size_t producerLimit = 8; // running on 8 core system.  Once we go over 7 producers it slows down.  That's one thing we want to see.
    static const size_t consumerLimit = 1;  // Just for documentation
    static const size_t messageCount = entryCount + consumerLimit +  producerLimit;

    static const size_t spinCount = 0;
    static const size_t yieldCount = ConsumerWaitStrategy::FOREVER;

    ConsumerWaitStrategy strategy(spinCount, yieldCount);
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    Consumer consumer(connection);
    Message consumerMessage(connection);

    for(size_t producerCount = 1; producerCount <= producerLimit; ++producerCount)
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
