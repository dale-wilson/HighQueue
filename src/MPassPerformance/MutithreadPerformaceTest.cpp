#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassPerformanceTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/IvProducer.h>
#include <InfiniteVector/IvConsumer.h>
#include <Common/Stopwatch.h>
#include <MPassPerformance/TestMessage.h>

using namespace MPass;
using namespace InfiniteVector;

namespace
{
    volatile std::atomic<uint32_t> producersWaiting = 0;
    volatile bool producersGo = false;

    void producerFunction(IvConnection & connection, uint32_t producerNumber, uint64_t messageCount)
    {
        IvProducer producer(connection);
        InfiniteVector::Message producerMessage;
        if(!connection.allocate(producerMessage))
        {
            std::cerr << "Failed to allocate message for producer Number " << producerNumber << std::endl;
            return;
        }

        ++producersWaiting;
        while(!producersGo)
        {
            std::this_thread::yield();
        }

        for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
        {
            auto testMessage = producerMessage.construct<TestMessage>(producerNumber, messageNumber);
            testMessage->touch();
            producer.publish(producerMessage);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMultithreadMessagePassingPerformance)
{
    static const size_t entryCount = 100000;
    static const size_t messageSize = sizeof(TestMessage);

    static const uint64_t targetMessageCount = 1000000 * 100; // runs about 5 to 10 seconds in release/optimized build
    static const size_t producerLimit = 10; // running on 8 core system.  Once we go over 7 producers it slows down.  That's one thing we want to see.
    static const size_t consumerLimit = 1;  // Just for documentation
    static const size_t messageCount = entryCount + consumerLimit +  producerLimit;

    static const size_t spinCount = 0;
    static const size_t yieldCount = IvConsumerWaitStrategy::FOREVER;

    IvConsumerWaitStrategy strategy(spinCount, yieldCount);
    IvCreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    IvConnection connection;
    connection.createLocal("LocalIv", parameters);

    IvConsumer consumer(connection);
    InfiniteVector::Message consumerMessage;
    BOOST_REQUIRE(connection.allocate(consumerMessage));

    for(size_t producerCount = 1; producerCount < producerLimit; ++producerCount)
    {
        std::cerr << "Test " << producerCount << " producer";

        std::vector<std::thread> producerThreads;
        std::vector<uint64_t> nextMessage;

        producersWaiting = 0;
        producersGo = false;
        size_t perProducer = targetMessageCount / producerCount;
        size_t actualMessageCount = perProducer * producerCount;

        for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
        {
            nextMessage.emplace_back(0u);
            producerThreads.emplace_back(
                std::bind(producerFunction, std::ref(connection), nTh, perProducer));
        }
        std::this_thread::yield();

        while(producersWaiting < producerCount)
        {
            std::this_thread::yield();
        }

        Stopwatch timer;
        producersGo = true;

        for(uint64_t messageNumber = 0; messageNumber < actualMessageCount; ++messageNumber)
        {
            consumer.getNext(consumerMessage);
            auto testMessage = consumerMessage.get<TestMessage>();
            testMessage->touch();
            auto producerNumber = testMessage->producerNumber_;
            auto & msgNumber = nextMessage[producerNumber];
            if(msgNumber != testMessage->messageNumber_)
            {
                // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
                BOOST_CHECK_EQUAL(messageNumber, testMessage->messageNumber_);
            }
            ++ msgNumber; 
        }

        auto lapse = timer.nanoseconds();

        // sometimes we synchronize thread shut down.
        producersGo = false;

        for(size_t nTh = 0; nTh < producerCount; ++nTh)
        {
            producerThreads[nTh].join();
        }

        auto messageBits = sizeof(TestMessage) * 8;
        std::cout << " Passed " << actualMessageCount << ' ' << messageBits << " bit messages in " 
            << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  " 
            << lapse / actualMessageCount << " nsec./message "
            << std::setprecision(2) << double(actualMessageCount * messageBits) / double(lapse) << " GBits/second."
            << std::endl;
    }
}
