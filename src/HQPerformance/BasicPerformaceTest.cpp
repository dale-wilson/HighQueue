#include <Common/HighQueuePch.hpp>
#define BOOST_TEST_NO_MAIN HighQueuePerformanceTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.hpp>
#include <HighQueue/Consumer.hpp>
#include <Mocks/MockMessage.hpp>
#include <Common/Stopwatch.hpp>

using namespace HighQueue;
typedef MockMessage<20> ActualMessage;

#define ENABLE_CONSUME_SEPARATELY 1
#if ENABLE_CONSUME_SEPARATELY
BOOST_AUTO_TEST_CASE(testPublishConsumeSeparately)
{
    WaitStrategy strategy;
    size_t entryCount = 10000000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messagesNeeded);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

    connection->willProduce(); // tell the connection that only one producer will use it.  This enables solo operation.
    Producer producer1(connection);
    Consumer consumer(connection);
    Message producerMessage(connection);
    Message consumerMessage(connection);

    Stopwatch timer;

    for(uint32_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        producerMessage.emplace<ActualMessage>(1, nMessage);
        producer1.publish(producerMessage);
    }
    auto publishSoloTime = timer.nanoseconds();
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
    auto consumeTime = timer.nanoseconds();

    connection->willProduce(); // tell the connection that another producer will use it.  This disables solo operation.
    Producer producer2(connection);
    for (uint32_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        producerMessage.emplace<ActualMessage>(1, nMessage);
        producer2.publish(producerMessage);
    }
    auto publishNotSolo = timer.nanoseconds();

    std::cout << entryCount << " messages. " << std::endl
        << "   Publish (solo)     " << publishSoloTime << " = " << publishSoloTime / entryCount << " nsec./msg " << std::endl
        << "   Publish (not solo) " << publishNotSolo << " = " << publishNotSolo / entryCount << " nsec./msg " << std::endl
        << "   Consume            " << consumeTime << " = " << consumeTime / entryCount << " nsec./msg" << std::endl;
    std::cout << std::endl;

}
#endif // ENABLECONSUME_SEPARATELY

#define ENABLE_NO_CONSUMER 1
#if ENABLE_NO_CONSUMER
BOOST_AUTO_TEST_CASE(testPublishWithNoConsumer)
{
#if defined(_DEBUG)
    uint32_t messageCount = 10;
#else // _DEBUG
    uint64_t messageCount = 100000000;
#endif // _DEBUG
    size_t producerCount = 1;

    WaitStrategy strategy;
    size_t entryCount = 1000;
    size_t messageSize = sizeof(ActualMessage);
    size_t messagesNeeded = entryCount + 10;
    bool discardMessagesIfNoConsumer = true;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messagesNeeded);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

    Producer producer1(connection);
    Message producerMessage(connection);

    Stopwatch timer;

    for(uint32_t nMessage = 0; nMessage < messageCount; ++nMessage)
    {
        producerMessage.emplace<ActualMessage>(1, nMessage);
        producer1.publish(producerMessage);
    }
    auto lapse = timer.nanoseconds();

    auto messageBytes = sizeof(ActualMessage);
    std::cout << "HighQueue Test with no Consumer: " << producerCount << " producer" << std::fixed;
    std::cout << " published " << messageCount << ' ' << messageBytes << " byte messages in "
        << std::setprecision(9) << double(lapse) / double(Stopwatch::nanosecondsPerSecond) << " seconds.  ";
    if(lapse == 0)
    {
        std::cout << "Run time too short to measure.   Use a larger messageCount" << std::endl;
    }
    else
    {
        std::cout
            << std::setprecision(3) << double(lapse) / double(messageCount) << " nsec./message "
            << std::setprecision(3) << double(messageCount * 1000) / double(lapse) << " MMsg/second "
            << std::endl;
    }
    std::cout << "No-consumer statistics:" << std::endl;
    producer1.writeStats(std::cout);
    std::cout << std::endl;
}
#endif // ENABLE_NO_CONSUMER
