#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassPerformanceTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/IvProducer.h>
#include <InfiniteVector/IvConsumer.h>
#include <MPassPerformance/TestMessage.h>
#include <Common/Stopwatch.h>

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
        connection.allocate(producerBuffer);
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

#define DISABLE_TWO_THREAD_PERFORMANCEx
#ifndef DISABLE_TWO_THREAD_PERFORMANCE
BOOST_AUTO_TEST_CASE(testTwoThreadPerformance)
{
    std::cerr << "Start 1 producer/1 consumer thread test." << std::endl;
    IvConsumerWaitStrategy strategy;
    size_t entryCount = 100000;
    size_t bufferSize = sizeof(TestMessage);
    size_t bufferCount = entryCount + 10;
    uint64_t messageCount = 1000000 * 100;

    IvCreationParameters parameters(strategy, entryCount, bufferSize, bufferCount);
    IvConnection connection;
    connection.createLocal("LocalIv", parameters);

    IvConsumer consumer(connection);
    Buffers::Buffer consumerBuffer;
    connection.allocate(consumerBuffer);

    producersWaiting = 0;
    producersGo = false;
    std::thread producerThread =
        std::thread(
            std::bind(producerFunction, connection, 0, messageCount));

    while(producersWaiting < 1)
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
        if(messageNumber != testMessage->messageNumber_)
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->messageNumber_);
        }
    }

    auto lapse = timer.nanoseconds();
    producerThread.join();

    auto messageBits = sizeof(TestMessage) * 8;
    std::cout << "Multithreaded: Passed " << messageCount << ' ' << messageBits << " bit messages in " << lapse << " nanoseconds.  " << lapse / messageCount << " nsec./message "
        << messageCount * 1000L * messageBits / lapse << " GBits/second."
        << std::endl;

}
#endif // DISABLE_TWO_THREAD_PERFORMANCE
