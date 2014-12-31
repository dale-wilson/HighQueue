#include "Common/MPassPch.h"

#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <ProntoQueue/Connection.h>
#include <ProntoQueue/details/PQResolver.h>
#include <ProntoQueue/details/PQReservePosition.h>

using namespace MPass;
using namespace ProntoQueue;

#define DISABLE_testIvConnectionMessagesx
#ifdef DISABLE_testIvConnectionMessages
#pragma message ("DISABLE_testIvConnectionMessages " __FILE__)
#else // DISABLE DISABLE_testIvConnectionMessages
BOOST_AUTO_TEST_CASE(testIvConnectionMessages)
{
    ConsumerWaitStrategy strategy;
    size_t entryCount = 100;
    size_t messageSize = 1234;
    size_t messageCount = 150;
    CreationParameters parameters(strategy, entryCount, messageSize, messageCount);
    Connection connection;
    connection.createLocal("LocalIv", parameters);

    BOOST_CHECK_LE(messageSize, connection.getMessageCapacity());
    BOOST_CHECK_LE(messageCount, connection.getMessageCount());

    ProntoQueue::Message message;
    for(size_t nMessage = 0; nMessage < (messageCount - entryCount); ++nMessage)
    {
        BOOST_CHECK(connection.hasMemoryAvailable());
        BOOST_CHECK(connection.allocate(message));
    }
    BOOST_CHECK(! connection.hasMemoryAvailable());
    BOOST_CHECK(!connection.allocate(message));

    // peek inside
    auto header = connection.getHeader();
    PQResolver resolver(header);
    auto readPosition = resolver.resolve<Position>(header->readPosition_);
    auto publishPosition = resolver.resolve<Position>(header->publishPosition_);
    auto reservePosition = resolver.resolve<PQReservePosition>(header->reservePosition_);
    BOOST_CHECK_EQUAL(*readPosition, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);
}
#endif //  DISABLE_testIvConnectionMessages

