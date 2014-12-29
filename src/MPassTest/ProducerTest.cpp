#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/IvProducer.h>

using namespace MPass;
using namespace InfiniteVector;

namespace
{
    struct TestMessage
    {
        char message_[100];
        TestMessage(const std::string & message)
        {
            size_t size = sizeof(message_);
            if(size > message.size())
            {
                size = message.size();
            }
            std::strncpy(message_, message.data(), size);
        }
    };
}


#define DISABLE_testProducerx
#ifdef DISABLE_testProducer
#pragma message ("DISABLE_testProducer " __FILE__)
#else // DISABLE_testProducer
BOOST_AUTO_TEST_CASE(testProducer)
{
    IvConsumerWaitStrategy strategy;
    size_t entryCount = 10;
    size_t bufferSize = sizeof(TestMessage);
    size_t bufferCount = 50;
    IvCreationParameters parameters(strategy, entryCount, bufferSize, bufferCount);
    IvConnection connection;
    connection.createLocal("LocalIv", parameters);
    IvProducer producer(connection);

    // peek inside the IV.
    auto header = connection.getHeader();
    IvResolver resolver(header);
    auto readPosition = resolver.resolve<Position>(header->readPosition_);
    auto publishPosition = resolver.resolve<Position>(header->publishPosition_);
    auto reservePosition = resolver.resolve<IvReservePosition>(header->reservePosition_);
    IvEntryAccessor accessor(resolver, header->entries_, header->entryCount_);


    Buffers::Buffer buffer;
    connection.allocate(buffer);
    auto testMessage = buffer.get<TestMessage>();
    new (testMessage) TestMessage("Hello world");
    buffer.setUsed(sizeof(TestMessage));
    BOOST_CHECK(!buffer.isEmpty());

    producer.publish(buffer);
    BOOST_CHECK(buffer.isEmpty());
    BOOST_CHECK(buffer.isValid());

    BOOST_CHECK_EQUAL(*readPosition + 1, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);


    IvEntry & firstEntry = accessor[*readPosition];
    BOOST_CHECK_EQUAL(firstEntry.status_, IvEntry::Status::OK);
    Buffers::Buffer & publishedBuffer = firstEntry.buffer_;
    auto publishedMessage = publishedBuffer.get<TestMessage>(); 
    auto publishedSize = publishedBuffer.getUsed();
    BOOST_CHECK_EQUAL(publishedMessage, testMessage);
    BOOST_CHECK_EQUAL(sizeof(TestMessage), publishedSize);
    
    for(size_t nMessage = 1; nMessage < entryCount; ++nMessage)
    {
        std::stringstream msg;
        msg << "Published " << nMessage << std::ends;
        new (buffer.get<TestMessage>()) TestMessage(msg.str());
        buffer.setUsed(sizeof(TestMessage));
        producer.publish(buffer);
    }
    // if we published another message now, it would hang.
    // todo: think of some way around that.

    BOOST_CHECK_EQUAL(*readPosition + entryCount, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);

    // Be sure the first message is still intact:
    publishedMessage = publishedBuffer.get<TestMessage>(); 
    publishedSize = publishedBuffer.getUsed();
    BOOST_CHECK_EQUAL(publishedMessage, testMessage);
    BOOST_CHECK_EQUAL(sizeof(TestMessage), publishedSize);

    // Simulate a consumer consuming the first message.
    ++(*readPosition);

    // Then publish one more
    auto fromTheTopMessage = buffer.get<TestMessage>();
    char topMessage[] = "Take it from the top";
    new (fromTheTopMessage) TestMessage("Take it from the top.");
    buffer.setUsed(sizeof(topMessage)); // not recommended for production!
    producer.publish(buffer);

    // Check to be sure that overwrote the first message.
    BOOST_CHECK_EQUAL(firstEntry.status_, IvEntry::Status::OK);
    Buffers::Buffer & newestBuffer = firstEntry.buffer_;
    auto newestMessage = newestBuffer.get<TestMessage>(); 
    auto newestSize = newestBuffer.getUsed();
    BOOST_CHECK_EQUAL(newestMessage, fromTheTopMessage);
    BOOST_CHECK_EQUAL(sizeof(topMessage), newestSize);
}
#endif //  DISABLE_testProducer

