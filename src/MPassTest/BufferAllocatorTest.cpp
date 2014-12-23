#include "Common/MPassPch.h"
#include <Buffers/BufferAllocator.h>
#include <Common/CacheLIne.h>
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

using namespace MPass;
using namespace Buffers;

BOOST_AUTO_TEST_CASE(testBufferAllocator)
{
    const static size_t oneLess = CacheLineSize - 1;
    const static size_t oneMore = CacheLineSize + 1;
    const static size_t bufferCount = 10;

    Buffer buffers[bufferCount + 5];
    
    BufferAllocator allocatorOneLess(oneLess, bufferCount);
    BOOST_CHECK_GE(allocatorOneLess.getBufferCount(), bufferCount);
    BOOST_CHECK_GE(allocatorOneLess.getBufferSize(), oneLess);
    for(size_t nBuffer = 0; nBuffer < allocatorOneLess.getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        BOOST_CHECK(allocatorOneLess.hasBuffers());
        BOOST_REQUIRE(allocatorOneLess.allocate(buffer));
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
        BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
        auto data = buffer.get<byte_t>();
        memset(data, '\0', allocatorOneLess.getBufferSize());
    }
    BOOST_CHECK(! allocatorOneLess.hasBuffers());
    BOOST_CHECK(! allocatorOneLess.allocate(buffers[bufferCount]) );
    for(size_t nBuffer = 0; nBuffer < allocatorOneLess.getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        allocatorOneLess.free(buffer);
        BOOST_CHECK(allocatorOneLess.hasBuffers());
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
    }
    
    BufferAllocator allocatorExact(CacheLineSize, bufferCount);
    BOOST_CHECK_GE(allocatorExact.getBufferCount(), bufferCount);
    BOOST_CHECK_GE(allocatorExact.getBufferSize(), oneLess);
    for(size_t nBuffer = 0; nBuffer < allocatorExact.getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        BOOST_CHECK(allocatorExact.hasBuffers());
        BOOST_REQUIRE(allocatorExact.allocate(buffer));
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
        BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
        auto data = buffer.get<byte_t>();
        memset(data, '\0', allocatorExact.getBufferSize());
    }
    BOOST_CHECK(! allocatorExact.hasBuffers());
    BOOST_CHECK(! allocatorExact.allocate(buffers[bufferCount]) );
    for(size_t nBuffer = 0; nBuffer < allocatorExact.getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        allocatorExact.free(buffer);
        BOOST_CHECK(allocatorExact.hasBuffers());
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
    }

    BufferAllocator allocatorOneMore(oneMore, bufferCount);
    BOOST_CHECK_GE(allocatorOneMore.getBufferCount(), bufferCount);
    BOOST_CHECK_GE(allocatorOneMore.getBufferSize(), oneLess);
    for(size_t nBuffer = 0; nBuffer < allocatorOneMore.getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        BOOST_CHECK(allocatorOneMore.hasBuffers());
        BOOST_REQUIRE(allocatorOneMore.allocate(buffer));
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
        BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
        auto data = buffer.get<byte_t>();
        memset(data, '\0', allocatorOneMore.getBufferSize());
    }
    BOOST_CHECK(! allocatorOneMore.hasBuffers());
    BOOST_CHECK(! allocatorOneMore.allocate(buffers[bufferCount]) );
    for(size_t nBuffer = 0; nBuffer < allocatorOneMore.getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        allocatorOneMore.free(buffer);
        BOOST_CHECK(allocatorOneMore.hasBuffers());
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
    }

    Buffer oops;
    BOOST_REQUIRE(allocatorOneLess.allocate(oops));
    BOOST_CHECK_THROW(allocatorExact.free(oops), std::runtime_error);

}
