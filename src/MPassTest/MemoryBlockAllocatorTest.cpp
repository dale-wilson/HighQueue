#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <Buffers/MemoryBlockHolder.h>
#include <Common/CacheLIne.h>

using namespace MPass;
using namespace Buffers;

BOOST_AUTO_TEST_CASE(testMemoryBlockAllocator)
{
    const static size_t oneLess = CacheLineSize - 1;
    const static size_t oneMore = CacheLineSize + 1;
    const static size_t bufferCount = 10;

    Buffer buffers[bufferCount + 5];
    
    MemoryBlockHolderPtr allocatorOneLess(std::make_shared<MemoryBlockHolder>(oneLess,bufferCount));
    BOOST_CHECK_GE(allocatorOneLess->getBufferCount(), bufferCount);
    BOOST_CHECK_GE(allocatorOneLess->getBufferCapacity(), oneLess);
    for(size_t nBuffer = 0; nBuffer < allocatorOneLess->getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        BOOST_CHECK(allocatorOneLess->hasMemoryAvailable());
        BOOST_REQUIRE(allocatorOneLess->allocate(buffer));
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
        BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
        auto data = buffer.get<byte_t>();
        memset(data, '\0', allocatorOneLess->getBufferCapacity());
    }
    BOOST_CHECK(! allocatorOneLess->hasMemoryAvailable());
    BOOST_CHECK(! allocatorOneLess->allocate(buffers[bufferCount]) );
    for(size_t nBuffer = 0; nBuffer < allocatorOneLess->getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        allocatorOneLess->release(buffer);
        BOOST_CHECK(allocatorOneLess->hasMemoryAvailable());
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
    }
    
    MemoryBlockHolderPtr allocatorExact(std::make_shared<MemoryBlockHolder>(CacheLineSize,bufferCount));
    BOOST_CHECK_GE(allocatorExact->getBufferCount(), bufferCount);
    BOOST_CHECK_GE(allocatorExact->getBufferCapacity(), oneLess);
    for(size_t nBuffer = 0; nBuffer < allocatorExact->getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        BOOST_CHECK(allocatorExact->hasMemoryAvailable());
        BOOST_REQUIRE(allocatorExact->allocate(buffer));
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
        BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
        auto data = buffer.get<byte_t>();
        memset(data, '\0', allocatorExact->getBufferCapacity());
    }
    BOOST_CHECK(! allocatorExact->hasMemoryAvailable());
    BOOST_CHECK(! allocatorExact->allocate(buffers[bufferCount]) );
    for(size_t nBuffer = 0; nBuffer < allocatorExact->getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        allocatorExact->release(buffer);
        BOOST_CHECK(allocatorExact->hasMemoryAvailable());
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
    }

    MemoryBlockHolderPtr allocatorOneMore(std::make_shared<MemoryBlockHolder>(oneMore,bufferCount));
    BOOST_CHECK_GE(allocatorOneMore->getBufferCount(), bufferCount);
    BOOST_CHECK_GE(allocatorOneMore->getBufferCapacity(), oneLess);
    for(size_t nBuffer = 0; nBuffer < allocatorOneMore->getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        BOOST_CHECK(allocatorOneMore->hasMemoryAvailable());
        BOOST_REQUIRE(allocatorOneMore->allocate(buffer));
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
        BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
        auto data = buffer.get<byte_t>();
        memset(data, '\0', allocatorOneMore->getBufferCapacity());
    }
    BOOST_CHECK(! allocatorOneMore->hasMemoryAvailable());
    BOOST_CHECK(! allocatorOneMore->allocate(buffers[bufferCount]) );
    for(size_t nBuffer = 0; nBuffer < allocatorOneMore->getBufferCount(); ++nBuffer)
    {
        Buffer & buffer(buffers[nBuffer]);
        allocatorOneMore->release(buffer);
        BOOST_CHECK(allocatorOneMore->hasMemoryAvailable());
        BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
    }

    Buffer oops;
    BOOST_REQUIRE(allocatorOneLess->allocate(oops));
    BOOST_CHECK_THROW(allocatorExact->release(oops), std::runtime_error);

}


BOOST_AUTO_TEST_CASE(testBufferOwner)
{
    const static size_t bufferSize = 100;
    const static size_t bufferCount = 5;

    MemoryBlockHolderPtr holder(std::make_shared<MemoryBlockHolder>(bufferSize, bufferCount));

    for(size_t nBuffer= 0;nBuffer < bufferCount * 10; ++nBuffer)
    {
        Buffer buffer;
        BOOST_REQUIRE(holder->allocate(buffer));
        (void)buffer.get();
    }
}

