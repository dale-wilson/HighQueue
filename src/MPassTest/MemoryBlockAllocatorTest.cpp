#include "Common/MPassPch.h"
#define BOOST_TEST_NO_MAIN MPassTest
#include <boost/test/unit_test.hpp>

#include <InfiniteVector/details/MemoryBlockPool.h>
#include <Common/CacheLIne.h>

using namespace MPass;
using namespace InfiniteVector;

#define DISABLE_testPoolAllocationx
#ifdef DISABLE_testPoolAllocation
#pragma message ("DISABLE_testPoolAllocation " __FILE__)
#else // DISABLE testPoolAllocation
BOOST_AUTO_TEST_CASE(testPoolAllocation)
{
    const static size_t messageSize = 100;
    const static size_t messageCount = 5;
    size_t blockSize = MemoryBlockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    auto pool = new (block.get()) MemoryBlockPool(blockSize, messageSize);
    BOOST_REQUIRE_GE(pool->getMessageCount(), messageCount);

    for(size_t nLoop = 0; nLoop < messageCount * 10; ++nLoop)
    {
        Message message1;
        BOOST_REQUIRE(pool->allocate(message1));
        Message message2;
        BOOST_REQUIRE(pool->allocate(message2));
        Message message3;
        BOOST_REQUIRE(pool->allocate(message3));
        Message message4;
        BOOST_REQUIRE(pool->allocate(message4));
        Message message5;
        BOOST_REQUIRE(pool->allocate(message5));
        (void)message5.get();
        message1.release();
        message5.release();
        message3.release();
        message2.release();
        message4.release();
    }
}
#endif // DISABLE testPoolAllocation

#define DISABLE_testAllocatorMessageOwnerx
#ifdef DISABLE_testAllocatorMessageOwner
#pragma message ("DISABLE_testAllocatorMessageOwner " __FILE__)
#else // DISABLE DISABLE_testAllocatorMessageOwner
BOOST_AUTO_TEST_CASE(testPoolAllocator)
{
    const static size_t messageSize = 100;
    const static size_t messageCount = 5;

    size_t blockSize = MemoryBlockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    auto pool = new (block.get()) MemoryBlockPool(blockSize, messageSize);
    for(size_t nLoop = 0; nLoop < messageCount * 10; ++nLoop)
    {
        Message message1;
        BOOST_REQUIRE(pool->allocate(message1));
        Message message2;
        BOOST_REQUIRE(pool->allocate(message2));
        Message message3;
        BOOST_REQUIRE(pool->allocate(message3));
        Message message4;
        BOOST_REQUIRE(pool->allocate(message4));
        Message message5;
        BOOST_REQUIRE(pool->allocate(message5));
        (void)message5.get();
    }
}
#endif // DISABLE_testAllocatorMessageOwner
