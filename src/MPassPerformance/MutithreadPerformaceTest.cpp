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
        uint64_t extra_[3];
        TestMessage(uint64_t data)
        :data_(data)
        {
            extra_[0] = extra_[1] = extra_[2] = data;
        }
        uint64_t touch() const
        {
            return data_ + extra_[0] + extra_[1] + extra_[2];
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
        auto testMessage = producerBuffer.construct<TestMessage>(messageNumber);
        testMessage->touch();
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
    uint64_t limit2 = 10000;
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
        testMessage->touch();
        if(messageNumber != testMessage->data_)
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->data_);
        }
    }

    auto lapse = timer.nanoseconds();
    producerThread.join();

    auto messageBits = sizeof(TestMessage) * 8;
    std::cout << "Multithreaded: Passed " << messageCount << ' ' << messageBits << " bit messages in " << lapse << " nanoseconds.  " << lapse / messageCount << " nsec./message "
        << messageCount * 1000L * messageBits / lapse << " GBits/second."
        << std::endl;

}
