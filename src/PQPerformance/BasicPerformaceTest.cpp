#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassPerformanceTest
#include <boost/test/unit_test.hpp>

#include <ProntoQueue/Producer.h>
#include <ProntoQueue/Consumer.h>
#include <PQPerformance/TestMessage.h>
#include <Common/Stopwatch.h>

using namespace MPass;
using namespace ProntoQueue;

#define DISABLE_CONSUME_SEPARATELYx
#ifndef DISABLE_CONSUME_SEPARATELY
BOOST_AUTO_TEST_CASE(testPublishConsumeSeparately)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 100000;
    size_t messageSize = sizeof(TestMessage);
    size_t messageCount = entryCount + 10;
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    Producer producer(connection);
    Consumer consumer(connection);
    ProntoQueue::Message producerMessage;
    connection.allocate(producerMessage);
    ProntoQueue::Message consumerMessage;
    connection.allocate(consumerMessage);

    Stopwatch timer;

    for(uint64_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        producerMessage.construct<TestMessage>(1, nMessage);
        producer.publish(producerMessage);
    }
    auto publishTime = timer.microseconds();
    timer.reset();
    // if we published another message now, it would hang.
    // todo: think of some way around that.

    // consume the messages.
    for(uint64_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        consumer.getNext(producerMessage);
        auto testMessage = producerMessage.get<TestMessage>();
        if(nMessage != testMessage->messageNumber_)
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(nMessage, testMessage->messageNumber_);
        }
    }
    auto consumeTime = timer.microseconds();

    std::cout << entryCount << " messages.  Publish " << publishTime * 1000 / entryCount << " nsec.  Consume " << consumeTime * 1000 / entryCount << " nsec." << std::endl;

}
#endif // DISABLE_CONSUME_SEPARATELY

#define DISABLE_ST_PERFORMANCEx
#ifndef DISABLE_ST_PERFORMANCE
BOOST_AUTO_TEST_CASE(testSingleThreadedMessagePassingPerformance)
{
    std::cerr << "Start producer and consumer in the same thread test." << std::endl;

    ConsumerWaitStrategy strategy;
    size_t entryCount = 100000;
    size_t messageSize = sizeof(TestMessage);
    size_t messagesNeeded = entryCount + 10;
    uint64_t messageCount = 1000000 * 100;

    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    Connection connection;
    connection.createLocal("LocalIv", parameters);


    Producer producer(connection);
    Consumer consumer(connection);
    ProntoQueue::Message producerMessage;
    connection.allocate(producerMessage);
    ProntoQueue::Message consumerMessage;
    connection.allocate(consumerMessage);

    Stopwatch timer;

    for(uint64_t messageNumber = 0; messageNumber < messageCount; ++messageNumber)
    {
        producerMessage.construct<TestMessage>(1, messageNumber);
        producer.publish(producerMessage);
        consumer.getNext(consumerMessage);
        auto testMessage = consumerMessage.get<TestMessage>();
        if(messageNumber != testMessage->messageNumber_)
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(messageNumber, testMessage->messageNumber_);
        }
    }
    auto lapse = timer.nanoseconds();

    auto messageBits = sizeof(TestMessage) * 8;
    std::cout << "Single threaded: Passed " << messageCount << ' ' << messageBits << " bit messages in "  
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(2) << double(messageCount * messageBits) / double(lapse) << " GBits/second."
        << std::endl;
}
#endif // DISABLE_ST_PERFORMANCE

