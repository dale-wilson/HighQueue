#include "Common/HSQueuePch.h"
#define BOOST_TEST_NO_MAIN HSQueueTest
#include <boost/test/unit_test.hpp>

#include <HSQueue/Producer.h>
#include <HSQueue/Consumer.h>

using namespace HSQueue;

namespace
{
    struct TestMessage
    {
        size_t size_;
        char message_[100];
        TestMessage(const std::string & message)
        {
            size_ = sizeof(message_);
            if(size_ > message.size())
            {
                size_ = message.size();
            }
            std::strncpy(message_, message.data(), size_);
        }
        std::string getString()const
        {
            return std::string(message_, size_);
        }
    };
}

#define DISABLE_testConsumerWithoutWaitsx
#ifdef DISABLE_testConsumerWithoutWaits
#pragma message ("DISABLE_testConsumerWithoutWaits " __FILE__)
#else // DISABLE DISABLE_testConsumerWithoutWaits
BOOST_AUTO_TEST_CASE(testConsumerWithoutWaits)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 10;
    size_t messageSize = sizeof(TestMessage);
    size_t messageCount = 50;
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    // We'll need these later.
    auto header = connection.getHeader();
    HSQResolver resolver(header);
    HSQEntryAccessor accessor(resolver, header->entries_, header->entryCount_);

    Producer producer(connection);
    HSQueue::Message message;
    connection.allocate(message);

    for(size_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        std::stringstream msg;
        msg << nMessage << std::ends;
        new (message.get<TestMessage>()) TestMessage(msg.str());
        message.setUsed(sizeof(TestMessage));
        producer.publish(message);
    }
    // if we published another message now, it would hang.
    // todo: think of some way around that.

    HSQueue::Message consumerMessage;
    connection.allocate(consumerMessage);

    // consume the messages.
    Consumer consumer(connection);
    for(size_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        std::stringstream msg;
        msg << nMessage << std::ends;

        consumer.getNext(message);
        BOOST_CHECK_EQUAL(sizeof(TestMessage), message.getUsed());
        auto testMessage = message.get<TestMessage>();
        BOOST_CHECK_EQUAL(msg.str(), testMessage->getString());                
    }

    BOOST_CHECK(! consumer.tryGetNext(message));
}
#endif //  DISABLE_testConsumerWithoutWaits
