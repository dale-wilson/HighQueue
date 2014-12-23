#include "Common/MPassPch.h"

#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/IvConnection.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvReservePosition.h>

using namespace MPass;
using namespace InfiniteVector;

BOOST_AUTO_TEST_CASE(testIvConnectionBuffers)
{
    IvConsumerWaitStrategy strategy;
    size_t entryCount = 100;
    size_t bufferSize = 1234;
    size_t bufferCount = 150;
    IvCreationParameters parameters(strategy, entryCount, bufferSize, bufferCount);
    IvConnection connection;
    connection.CreateLocal("LocalIv", parameters);

    BOOST_CHECK_LE(bufferSize, connection.getBufferSize());
    BOOST_CHECK_LE(bufferCount, connection.getBufferCount());

    Buffers::Buffer buffer;
    for(size_t nBuffer = 0; nBuffer < (bufferCount - entryCount); ++nBuffer)
    {
        BOOST_CHECK(connection.hasBuffers());
        BOOST_CHECK(connection.allocate(buffer));
    }
    BOOST_CHECK(! connection.hasBuffers());
    BOOST_CHECK(!connection.allocate(buffer));

    // peek inside
    auto header = connection.getHeader();
    IvResolver resolver(header);
    auto readPosition = resolver.resolve<Position>(header->readPosition_);
    auto publishPosition = resolver.resolve<Position>(header->publishPosition_);
    auto reservePosition = resolver.resolve<IvReservePosition>(header->reservePosition_);
    BOOST_CHECK_EQUAL(*readPosition, *publishPosition);
    BOOST_CHECK_EQUAL(*publishPosition, reservePosition->reservePosition_);

}
