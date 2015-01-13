#include "Common/HighQueuePch.h"
#define BOOST_TEST_NO_MAIN HighQueueTest
#include <boost/test/unit_test.hpp>

#include <HighQueue/details/HQMemoryBlockPool.h>
#include <Common/CacheLIne.h>

using namespace HighQueue;

#define DISABLE_testPoolAllocationx
#ifdef DISABLE_testPoolAllocation
#pragma message ("DISABLE_testPoolAllocation " __FILE__)
#else // DISABLE testPoolAllocation
BOOST_AUTO_TEST_CASE(testPoolAllocation)
{
    const static size_t messageSize = 100;
    const static size_t messageCount = 5;
    size_t blockSize = HQMemoryBlockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    auto pool = new (block.get()) HQMemoryBlockPool(blockSize, messageSize);
    BOOST_REQUIRE_GE(pool->getBlockCount(), messageCount);

    for(size_t nLoop = 0; nLoop < messageCount * 10; ++nLoop)
    {
        Message message1(pool);
        Message message2(pool);
        Message message3(pool);
        Message message4(pool);
        Message message5(pool);
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

    size_t blockSize = HQMemoryBlockPool::spaceNeeded(messageSize, messageCount);
    std::unique_ptr<byte_t> block(new byte_t[blockSize]);
    auto pool = new (block.get()) HQMemoryBlockPool(blockSize, messageSize);
    for(size_t nLoop = 0; nLoop < messageCount * 10; ++nLoop)
    {
        Message message1(pool);
        Message message2(pool);
        Message message3(pool);
        Message message4(pool);
        Message message5(pool);
        (void)message5.get();
    }
}
#endif // DISABLE_testAllocatorMessageOwner
