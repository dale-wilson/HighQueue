#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>
#include <HighQueue/Consumer.h>
#include <Mocks/TestMessage.h>
#include <Common/Stopwatch.h>

using namespace HighQueue;
typedef TestMessage<20> ActualMessage;

#define ENABLE_CONSUME_SEPARATELY 1
#if ENABLE_CONSUME_SEPARATELY
BOOST_AUTO_TEST_CASE(testPublishConsumeSeparately)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 100000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messageCount = entryCount + 10;
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

    Producer producer(connection);
    Consumer consumer(connection);
    Message producerMessage(connection);
    Message consumerMessage(connection);

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
        if(nMessage != testMessage->getSequence())
        {
            // the if avoids the performance hit of BOOST_CHECK_EQUAL unless it's needed.
            BOOST_CHECK_EQUAL(nMessage, testMessage->getSequence());
        }
    }
    auto consumeTime = timer.microseconds();

    std::cout << entryCount << " messages.  Publish " << publishTime * 1000 / entryCount << " nsec.  Consume " << consumeTime * 1000 / entryCount << " nsec." << std::endl;

}
#endif // ENABLECONSUME_SEPARATELY

