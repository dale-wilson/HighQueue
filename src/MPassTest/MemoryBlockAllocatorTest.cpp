#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <Buffers/MemoryBlockPool.h>
#include <Common/CacheLIne.h>

using namespace MPass;
using namespace Buffers;

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
    auto pool = new (block.get()) MemoryBlockPool(blockSize, bufferSize);
    BOOST_REQUIRE_GE(pool->getBufferCount(), bufferCount);

    for(size_t nLoop = 0; nLoop < bufferCount * 10; ++nLoop)
    {
        Buffer buffer1;
        BOOST_REQUIRE(pool->allocate(buffer1));
        Buffer buffer2;
        BOOST_REQUIRE(pool->allocate(buffer2));
        Buffer buffer3;
        BOOST_REQUIRE(pool->allocate(buffer3));
        Buffer buffer4;
        BOOST_REQUIRE(pool->allocate(buffer4));
        Buffer buffer5;
        BOOST_REQUIRE(pool->allocate(buffer5));
        (void)buffer5.get();
        buffer1.release();
        buffer5.release();
        buffer3.release();
        buffer2.release();
        buffer4.release();
    }
}
#endif // DISABLE testPoolAllocation

#define DISABLE_testAllocatorBufferOwnerx
#ifdef DISABLE_testAllocatorBufferOwner
#pragma message ("DISABLE_testAllocatorBufferOwner " __FILE__)
#else // DISABLE DISABLE_testAllocatorBufferOwner
BOOST_AUTO_TEST_CASE(testPoolAllocator)
{
    const static size_t bufferSize = 100;
    const static size_t bufferCount = 5;

    size_t blockSize = MemoryBlockPool::spaceNeeded(bufferSize, bufferCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    auto pool = new (block.get()) MemoryBlockPool(blockSize, bufferSize);
    for(size_t nLoop = 0; nLoop < bufferCount * 10; ++nLoop)
    {
        Buffer buffer1;
        BOOST_REQUIRE(pool->allocate(buffer1));
        Buffer buffer2;
        BOOST_REQUIRE(pool->allocate(buffer2));
        Buffer buffer3;
        BOOST_REQUIRE(pool->allocate(buffer3));
        Buffer buffer4;
        BOOST_REQUIRE(pool->allocate(buffer4));
        Buffer buffer5;
        BOOST_REQUIRE(pool->allocate(buffer5));
        (void)buffer5.get();
    }
}
#endif // DISABLE_testAllocatorBufferOwner
