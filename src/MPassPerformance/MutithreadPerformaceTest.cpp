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
        Buffers::Buffer producerBuffer;
        if(!connection.allocate(producerBuffer))
        {
            std::cerr << "Failed to allocate buffer for producer Number " << producerNumber << std::endl;
            return;
        }
        //{
        //    std::stringstream msg;
        //    msg << "Start producer " <<producerNumber << " in thread " << std::this_thread::get_id() << std::endl;
        //    std::cerr << msg.str() << std::flush;
        //}
        ++producersWaiting;
        while(!producersGo)
        {
            std::this_thread::yield();
        }

        for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
        {
            auto testMessage = producerBuffer.construct<TestMessage>(producerNumber, messageNumber);
            testMessage->touch();
            producer.publish(producerBuffer);
        }

        //{
        //    std::stringstream msg;
        //    msg << "ready to Exit " << producerNumber << " T#: " << std::this_thread::get_id() << std::endl;
        //    std::cerr << msg.str() << std::flush;
        //}

    }
}

BOOST_AUTO_TEST_CASE(testMultithreadMessagePassingPerformance)
{
    IvConsumerWaitStrategy strategy;
    static const size_t entryCount = 100000;
    static const size_t bufferSize = sizeof(TestMessage);

    static const uint64_t perProducer = 1000000 * 10; // runs about 5 seconds in release/optimized build
    static const size_t producerCount = 7; // running on 8 core system.
    static const size_t bufferCount = entryCount + producerCount + 1;

    static const uint64_t messageCount = perProducer * producerCount;

    std::cerr << "Start "<< producerCount << " producer/1 consumer thread test." << std::endl;

    IvCreationParameters parameters(strategy, entryCount, bufferSize, bufferCount);
    IvConnection connection;
    connection.createLocal("LocalIv", parameters);

    IvConsumer consumer(connection);
    Buffers::Buffer consumerBuffer;
    BOOST_REQUIRE(connection.allocate(consumerBuffer));


    std::thread producerThreads[producerCount + 1];
    uint64_t nextMessage[producerCount + 1];
    producersWaiting = 0;
    producersGo = false;

    for(uint32_t nTh = 0; nTh < producerCount; ++nTh)
    {
        nextMessage[nTh] = 0u;
        producerThreads[nTh] = std::thread(
            std::bind(producerFunction, std::ref(connection), nTh, perProducer));
    }
    std::this_thread::yield();

    while(producersWaiting < producerCount)
    {
        std::this_thread::yield();
    }
    Stopwatch timer;
    producersGo = true;

    for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
    {
        consumer.getNext(consumerBuffer);
        auto testMessage = consumerBuffer.get<TestMessage>();
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

    producersGo = false;

    for(size_t nTh = 0; nTh < producerCount; ++nTh)
    {
        producerThreads[nTh].join();
    }

    auto messageBits = sizeof(TestMessage) * 8;
    std::cout << "Multithreaded: " << producerCount  << " producers. Passed " << messageCount << ' ' << messageBits << " bit messages in " << lapse << " nanoseconds.  " << lapse / messageCount << " nsec./message "
        << messageCount * 1000L * messageBits / lapse << " GBits/second."
        << std::endl;

}
