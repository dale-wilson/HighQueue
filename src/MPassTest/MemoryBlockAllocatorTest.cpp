#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <Buffers/MemoryBlockHolder.h>
#include <Common/CacheLIne.h>

using namespace MPass;
using namespace Buffers;

#define DISABLE_testMemoryBlockHolderx
#ifdef DISABLE_testMemoryBlockHolder
#pragma message ("DISABLE_testMemoryBlockHolder " __FILE__)
#else // DISABLE DISABLE_testMemoryBlockHolder
BOOST_AUTO_TEST_CASE(testMemoryBlockHolder)
{
    const static size_t oneLess = CacheLineSize - 1;
    const static size_t oneMore = CacheLineSize + 1;
    const static size_t bufferCount = 10;

    Buffer buffers[bufferCount + 5];
    {
        MemoryBlockHolderPtr holderOneLess(std::make_shared<MemoryBlockHolder>(oneLess,bufferCount));
        BOOST_CHECK_GE(holderOneLess->getBufferCount(), bufferCount);
        BOOST_CHECK_GE(holderOneLess->getBufferCapacity(), oneLess);
        for(size_t nBuffer = 0; nBuffer < holderOneLess->getBufferCount(); ++nBuffer)
        {
            Buffer & buffer(buffers[nBuffer]);
            BOOST_CHECK(! holderOneLess->isEmpty());
            BOOST_REQUIRE(holderOneLess->allocate(buffer));
            BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
            BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
            auto data = buffer.get<byte_t>();
            memset(data, '\0', holderOneLess->getBufferCapacity());
        }
        BOOST_CHECK(holderOneLess->isEmpty());
        BOOST_CHECK(! holderOneLess->allocate(buffers[bufferCount]) );
        for(size_t nBuffer = 0; nBuffer < holderOneLess->getBufferCount(); ++nBuffer)
        {
            Buffer & buffer(buffers[nBuffer]);
            holderOneLess->release(buffer);
            BOOST_CHECK(! holderOneLess->isEmpty());
            BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
        }
    
        MemoryBlockHolderPtr holderExact(std::make_shared<MemoryBlockHolder>(CacheLineSize,bufferCount));
        BOOST_CHECK_GE(holderExact->getBufferCount(), bufferCount);
        BOOST_CHECK_GE(holderExact->getBufferCapacity(), oneLess);
        for(size_t nBuffer = 0; nBuffer < holderExact->getBufferCount(); ++nBuffer)
        {
            Buffer & buffer(buffers[nBuffer]);
            BOOST_CHECK(!holderExact->isEmpty());
            BOOST_REQUIRE(holderExact->allocate(buffer));
            BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
            BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
            auto data = buffer.get<byte_t>();
            memset(data, '\0', holderExact->getBufferCapacity());
        }
        BOOST_CHECK(holderExact->isEmpty());
        BOOST_CHECK(! holderExact->allocate(buffers[bufferCount]) );
        for(size_t nBuffer = 0; nBuffer < holderExact->getBufferCount(); ++nBuffer)
        {
            Buffer & buffer(buffers[nBuffer]);
            holderExact->release(buffer);
            BOOST_CHECK(!holderExact->isEmpty());
            BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
        }

        MemoryBlockHolderPtr holderOneMore(std::make_shared<MemoryBlockHolder>(oneMore,bufferCount));
        BOOST_CHECK_GE(holderOneMore->getBufferCount(), bufferCount);
        BOOST_CHECK_GE(holderOneMore->getBufferCapacity(), oneLess);
        for(size_t nBuffer = 0; nBuffer < holderOneMore->getBufferCount(); ++nBuffer)
        {
            Buffer & buffer(buffers[nBuffer]);
            BOOST_CHECK(!holderOneMore->isEmpty());
            BOOST_REQUIRE(holderOneMore->allocate(buffer));
            BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Normal);
            BOOST_CHECK_EQUAL(buffer.getUsed(), 0U);
            auto data = buffer.get<byte_t>();
            memset(data, '\0', holderOneMore->getBufferCapacity());
        }
        BOOST_CHECK(holderOneMore->isEmpty());
        BOOST_CHECK(! holderOneMore->allocate(buffers[bufferCount]) );
        for(size_t nBuffer = 0; nBuffer < holderOneMore->getBufferCount(); ++nBuffer)
        {
            Buffer & buffer(buffers[nBuffer]);
            holderOneMore->release(buffer);
            BOOST_CHECK(!holderOneMore->isEmpty());
            BOOST_CHECK_EQUAL(buffer.getType(), Buffer::Type::Invalid);
        }

        Buffer oops;
        BOOST_REQUIRE(holderOneLess->allocate(oops));
        BOOST_CHECK_THROW(holderExact->release(oops), std::runtime_error);
    }
}
#endif // DISABLE_testMemoryBlockHolder

#define DISABLE_testPoolAllocationx
#ifdef DISABLE_testPoolAllocation
#pragma message ("DISABLE_testPoolAllocation " __FILE__)
#else // DISABLE testPoolAllocation
BOOST_AUTO_TEST_CASE(testPoolAllocation)
{
    const static size_t bufferSize = 100;
    const static size_t bufferCount = 5;

    size_t blockSize = MemoryBlockPool::spaceNeeded(bufferSize, bufferCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    MemoryBlockPool pool(block.get(), blockSize, MemoryBlockPool::cacheAlignedBufferSize(bufferSize));
    Buffer::MemoryOwnerPtr nobody;

    for(size_t nLoop = 0; nLoop < bufferCount * 10; ++nLoop)
    {
        Buffer buffer1;
        BOOST_REQUIRE(pool.allocate(block.get(), buffer1, nobody));
        Buffer buffer2;
        BOOST_REQUIRE(pool.allocate(block.get(), buffer2, nobody));
        Buffer buffer3;
        BOOST_REQUIRE(pool.allocate(block.get(), buffer3, nobody));
        Buffer buffer4;
        BOOST_REQUIRE(pool.allocate(block.get(), buffer4, nobody));
        Buffer buffer5;
        BOOST_REQUIRE(pool.allocate(block.get(), buffer5, nobody));
        (void)buffer5.get();
        pool.release(block.get(), buffer1);
        pool.release(block.get(), buffer5);
        pool.release(block.get(), buffer3);
        pool.release(block.get(), buffer2);
        pool.release(block.get(), buffer4);
    }
}
#endif // DISABLE testPoolAllocation

#define DISABLE_testAllocatorBufferOwnerx
#ifdef DISABLE_testAllocatorBufferOwner
#pragma message ("DISABLE_testAllocatorBufferOwner " __FILE__)
#else // DISABLE DISABLE_testAllocatorBufferOwner
BOOST_AUTO_TEST_CASE(testAllocatorBufferOwner)
{
    const static size_t bufferSize = 100;
    const static size_t bufferCount = 5;

    size_t blockSize = MemoryBlockPool::spaceNeeded(bufferSize, bufferCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    MemoryBlockPool pool(block.get(), blockSize, MemoryBlockPool::cacheAlignedBufferSize(bufferSize));
    MemoryBlockAllocatorPtr allocator(std::make_shared<MemoryBlockAllocator>(block.get(), pool));

    for(size_t nLoop = 0; nLoop < bufferCount * 10; ++nLoop)
    {
        Buffer buffer1;
        BOOST_REQUIRE(allocator->allocate(buffer1));
        Buffer buffer2;
        BOOST_REQUIRE(allocator->allocate(buffer2));
        Buffer buffer3;
        BOOST_REQUIRE(allocator->allocate(buffer3));
        Buffer buffer4;
        BOOST_REQUIRE(allocator->allocate(buffer4));
        Buffer buffer5;
        BOOST_REQUIRE(allocator->allocate(buffer5));
        (void)buffer5.get();
    }
}
#endif // DISABLE_testAllocatorBufferOwner

#define DISABLE_testHolderBufferOwnerx
#ifdef DISABLE_testHolderBufferOwner
#pragma message ("DISABLE_testHolderBufferOwner " __FILE__)
#else // DISABLE testPoolAllocation
BOOST_AUTO_TEST_CASE(testHolderBufferOwner)
{
    const static size_t bufferSize = 100;
    const static size_t bufferCount = 5;

    MemoryBlockHolderPtr holder(std::make_shared<MemoryBlockHolder>(bufferSize, bufferCount));

    for(size_t nLoop= 0; nLoop < bufferCount * 10; ++nLoop)
    {
        Buffer buffer1;
        BOOST_REQUIRE(holder->allocate(buffer1));
        Buffer buffer2;
        BOOST_REQUIRE(holder->allocate(buffer2));
        Buffer buffer3;
        BOOST_REQUIRE(holder->allocate(buffer3));
        Buffer buffer4;
        BOOST_REQUIRE(holder->allocate(buffer4));
        Buffer buffer5;
        BOOST_REQUIRE(holder->allocate(buffer5));
        (void)buffer5.get();
    }
}
#endif // DISABLE_testHolderBufferOwner

