#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/IvProducer.h>
#include <InfiniteVector/IvConsumer.h>

using namespace MPass;
using namespace InfiniteVector;

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

BOOST_AUTO_TEST_CASE(testConsumerWithoutWaits)
{
    IvConsumerWaitStrategy strategy;
    size_t entryCount = 10;
    size_t bufferSize = sizeof(TestMessage);
    size_t bufferCount = 50;
    IvCreationParameters parameters(strategy, entryCount, bufferSize, bufferCount);
    IvConnection connection;
    connection.createLocal("LocalIv", parameters);

    // We'll need these later.
    auto header = connection.getHeader();
    IvResolver resolver(header);
    IvEntryAccessor accessor(resolver, header->entries_, header->entryCount_);

    IvProducer producer(connection);
    Buffers::Buffer buffer;
    connection.allocate(buffer);

    for(size_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        std::stringstream msg;
        msg << nMessage << std::ends;
        new (buffer.get<TestMessage>()) TestMessage(msg.str());
        buffer.setUsed(sizeof(TestMessage));
        producer.publish(buffer);
    }
    // if we published another message now, it would hang.
    // todo: think of some way around that.

    Buffers::Buffer consumerBuffer;
    connection.allocate(consumerBuffer);

    // consume the messages.
    IvConsumer consumer(connection);
    for(size_t nMessage = 0; nMessage < entryCount; ++nMessage)
    {
        std::stringstream msg;
        msg << nMessage << std::ends;

        consumer.getNext(buffer);
        BOOST_CHECK_EQUAL(sizeof(TestMessage), buffer.getUsed());
        auto testMessage = buffer.get<TestMessage>();
        BOOST_CHECK_EQUAL(msg.str(), testMessage->getString());                
    }

    BOOST_CHECK(! consumer.tryGetNext(buffer));

}
