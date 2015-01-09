#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>
#include <HighQueue/Consumer.h>
#include <HQPerformance/TestMessage.h>
#include <Common/Stopwatch.h>

using namespace HighQueue;
typedef TestMessage<20> ActualMessage;

#define DISABLE_CONSUME_SEPARATELYx
#ifndef DISABLE_CONSUME_SEPARATELY
BOOST_AUTO_TEST_CASE(testPublishConsumeSeparately)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 100000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messageCount = entryCount + 10;
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    Producer producer(connection);
    Consumer consumer(connection);
    Message producerMessage;
    connection.allocate(producerMessage);
    Message consumerMessage;
    connection.allocate(consumerMessage);

    Stopwatch timer;

    for(uint32_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        producerMessage.emplace<ActualMessage>(1, nMessage);
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
        auto testMessage = producerMessage.get<ActualMessage>();
        if(nMessage != testMessage->messageNumber())
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(nMessage, testMessage->messageNumber());
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
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    uint64_t messageCount = 1000000 * 100;

    CreationParameters parameters(strategy, entryCount, messageSize, messagesNeeded);
    Connection connection;
    connection.createLocal("LocalIv", parameters);


    Producer producer(connection);
    Consumer consumer(connection);
    Message producerMessage;
    connection.allocate(producerMessage);
    Message consumerMessage;
    connection.allocate(consumerMessage);

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
    std::cout << "Single threaded: Passed " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  "
        << lapse / messageCount << " nsec./message "
        << std::setprecision(3) << double(messageCount) / double(lapse) << " GMsg/second "
        << std::setprecision(3) << double(messageCount * messageBytes) / double(lapse) << " GByte/second "
        << std::setprecision(3) << double(messageCount * messageBits) / double(lapse) << " GBit/second."
        << std::endl;
}
#endif // DISABLE_ST_PERFORMANCE

