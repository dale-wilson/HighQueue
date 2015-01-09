#include "Common/HighQueuePch.h"

#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/Connection.h>
#include <HighQueue/details/HQResolver.h>
#include <HighQueue/details/HQReservePosition.h>

using namespace HighQueue;

#define DISABLE_testIvMemoryPoolMessagesx
#ifdef DISABLE_testIvMemoryPoolMessages
#pragma message ("DISABLE_testIvMemoryPoolMessages " __FILE__)
#else // DISABLE DISABLE_testIvMemoryPoolMessages
BOOST_AUTO_TEST_CASE(testIvMemoryPoolMessages)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 100;
    size_t messageSize = 1234;
    size_t messageCount = 150;
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    BOOST_CHECK_LE(messageSize, connection.getMessageCapacity());

    Message message;
    for(size_t nMessage = 0; nMessage < (messageCount - entryCount); ++nMessage)
    {
        BOOST_CHECK(connection.allocate(message));
    }
    BOOST_CHECK(!connection.allocate(message));

    // peek inside
    auto header = connection.getHeader();
    HighQResolver resolver(header);
    auto readPosition = resolver.resolve<Position>(header->readPosition_);
    auto publishPosition = resolver.resolve<Position>(header->publishPosition_);
    auto reservePosition = resolver.resolve<HighQReservePosition>(header->reservePosition_);
    BOOST_CHECK_EQUAL(*readPosition, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);
}
#endif //  DISABLE_testIvMemoryPoolMessages

