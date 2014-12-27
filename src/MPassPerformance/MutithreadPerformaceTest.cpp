#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassPerformanceTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/IvProducer.h>
#include <InfiniteVector/IvConsumer.h>
#include <Common/Stopwatch.h>

using namespace MPass;
using namespace InfiniteVector;

namespace
{
    struct TestMessage
    {
        uint64_t data_;
        TestMessage(uint64_t data)
        :data_(data)
        {
        }
    };

    volatile bool producerIsWaiting = false;
    std::mutex startMutex;
}

void producerFunction(IvConnection connection, uint64_t messageCount)
{
    IvProducer producer(connection);
    Buffers::Buffer producerBuffer;
    connection.allocate(producerBuffer);
    producerIsWaiting = true;
    {
        std::lock_guard<std::mutex> guard(startMutex);
        producerIsWaiting = false;
    }
    for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
    {
        producerBuffer.construct<TestMessage>(messageNumber);
        producer.publish(producerBuffer);
    }
}

BOOST_AUTO_TEST_CASE(testMultithreadMessagePassingPerformance)
{
    IvConsumerWaitStrategy strategy;
    size_t entryCount = 100000;
    size_t bufferSize = sizeof(TestMessage);
    size_t bufferCount = entryCount + 10;

    uint64_t limit1 = 100000;
    uint64_t limit2 = 1000;
    uint64_t messageCount = limit1 * limit2;

    IvCreationParameters parameters(strategy, entryCount, bufferSize, bufferCount);
    IvConnection connection;
    connection.CreateLocal("LocalIv", parameters);

    IvConsumer consumer(connection);
    Buffers::Buffer consumerBuffer;
    connection.allocate(consumerBuffer);

    std::thread producerThread;
    Stopwatch timer;
    {
        producerIsWaiting = false;
        std::lock_guard<std::mutex> guard(startMutex);
        producerThread = std::thread(
            std::bind(producerFunction, connection, messageCount));
        while(!producerIsWaiting)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        timer.reset();
    }

    for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
    {
        consumer.getNext(consumerBuffer);
        auto testMessage = consumerBuffer.get<TestMessage>();
        if(messageNumber != testMessage->data_)
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->data_);
        }
    }

    auto lapse = timer.nanoseconds();
    producerThread.join();
    std::cout << "Multithreaded: Passed " << messageCount << " messages in " << lapse << " nanoseconds.  " << lapse / messageCount << " nsec./message " 
        << messageCount * 1000L * sizeof(TestMessage) * 8 / lapse << " GBits/second."
        << std::endl;

}
