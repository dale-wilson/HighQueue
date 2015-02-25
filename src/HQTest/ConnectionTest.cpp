#include <Common/HighQueuePch.hpp>

#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Connection.hpp>
#include <HighQueue/details/HQResolver.hpp>
#include <HighQueue/details/HQReservePosition.hpp>

using namespace HighQueue;

#define DISABLE_testIvMemoryPoolMessagesx
#ifdef DISABLE_testIvMemoryPoolMessages
#pragma message ("DISABLE_testIvMemoryPoolMessages " __FILE__)
#else // DISABLE DISABLE_testIvMemoryPoolMessages
BOOST_AUTO_TEST_CASE(testIvMemoryPoolMessages)
{
    WaitStrategy strategy;
    size_t entryCount = 100;
    size_t messageSize = 1234;
    size_t messageCount = 150;
    bool discardMessagesIfNoConsumer = false;
    CreationParameters parameters(strategy, strategy, discardMessagesIfNoConsumer, entryCount, messageSize, messageCount);
    ConnectionPtr connection = std::make_shared<Connection>();
    connection->createLocal("LocalIv", parameters);

    BOOST_CHECK_LE(messageSize, connection->getMessageCapacity());

    //std::vector<Message> messages;
    //for(size_t nMessage = 0; nMessage < (messageCount - entryCount); ++nMessage)
    //{
    //    messages.emplace_back(connection);
    //}

    // peek inside
    auto header = connection->getHeader();
    HighQResolver resolver(header);
    auto readPosition = resolver.resolve<Position>(header->readPosition_);
    auto publishPosition = resolver.resolve<Position>(header->publishPosition_);
    auto reservePosition = resolver.resolve<HighQReservePosition>(header->reservePosition_);
    BOOST_CHECK_EQUAL(*readPosition, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);
}
#endif //  DISABLE_testIvMemoryPoolMessages

