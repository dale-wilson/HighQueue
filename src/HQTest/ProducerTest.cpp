#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Producer.h>

using namespace HighQueue;

namespace
{
    struct MockMessage
    {
        char message_[100];
        MockMessage(const std::string & text)
        {
            size_t size = sizeof(message_);
            if(size > text.size())
            {
                size = text.size();
            }
            std::strncpy(message_, text.data(), size);
        }
    };
}


#define DISABLE_testProducerx
#ifdef DISABLE_testProducer
#pragma message ("DISABLE_testProducer " __FILE__)
#else // DISABLE_testProducer
BOOST_AUTO_TEST_CASE(testProducer)
{
    WaitStrategy strategy;
    size_t entryCount = 10;
    size_t messageSize = sizeof(MockMessage);
    size_t messageCount = 50;
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messageCount);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);
    Producer producer(connection);
    // peek inside the IV.
    auto header = connection->getHeader();
    HighQResolver resolver(header);
    auto readPosition = resolver.resolve<Position>(header->readPosition_);
    auto publishPosition = resolver.resolve<Position>(header->publishPosition_);
    auto reservePosition = resolver.resolve<HighQReservePosition>(header->reservePosition_);
    HighQEntryAccessor accessor(resolver, header->entries_, header->entryCount_);


    Message message(connection);
    auto testMessage = message.get<MockMessage>();
    new (testMessage) MockMessage("Hello world");
    message.setUsed(sizeof(MockMessage));
    BOOST_CHECK(!message.isEmpty());

    producer.publish(message);
    BOOST_CHECK(message.isEmpty());

    BOOST_CHECK_EQUAL(*readPosition + 1, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);


    HighQEntry & firstEntry = accessor[*readPosition];
    BOOST_CHECK_EQUAL(firstEntry.status_, HighQEntry::Status::OK);
    Message & firstMessage = firstEntry.message_;
    auto publishedMessage = firstMessage.get<MockMessage>(); 
    auto publishedSize = firstMessage.getUsed();
    BOOST_CHECK_EQUAL(publishedMessage, testMessage);
    BOOST_CHECK_EQUAL(sizeof(MockMessage), publishedSize);
    
    for(size_t nMessage = 1; nMessage < entryCount; ++nMessage)
    {
        std::stringstream msg;
        msg << "Published " << nMessage << std::ends;
        new (message.get<MockMessage>()) MockMessage(msg.str());
        message.setUsed(sizeof(MockMessage));
        producer.publish(message);
    }
    // if we published another message now, it would hang.
    // todo: think of some way around that.

    BOOST_CHECK_EQUAL(*readPosition + entryCount, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);

    // Be sure the first message is still intact:
    publishedMessage = firstMessage.get<MockMessage>(); 
    publishedSize = firstMessage.getUsed();
    BOOST_CHECK_EQUAL(publishedMessage, testMessage);
    BOOST_CHECK_EQUAL(sizeof(MockMessage), publishedSize);

    // Simulate a consumer consuming the first message.
    ++(*readPosition);

    // Then publish one more
    // this technique is for testing, it is not recommended for production!
    // use the construct method
    auto fromTheTopMessage = message.get<MockMessage>();
    char topMessage[] = "Take it from the top";
    new (fromTheTopMessage) MockMessage("Take it from the top.");
    message.setUsed(sizeof(topMessage));     
    producer.publish(message);

    // Check to be sure that overwrote the first message.
    BOOST_CHECK_EQUAL(firstEntry.status_, HighQEntry::Status::OK);
    Message & newestMessage = firstEntry.message_;
    auto newestMockMessage = newestMessage.get<MockMessage>(); 
    auto newestSize = newestMessage.getUsed();
    BOOST_CHECK_EQUAL(newestMockMessage, fromTheTopMessage);
    BOOST_CHECK_EQUAL(sizeof(topMessage), newestSize);
}
#endif //  DISABLE_testProducer

