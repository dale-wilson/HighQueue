#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockAllocator.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockAllocator::MemoryBlockAllocator(byte_t * baseAddress, MemoryBlockPool & pool)
: baseAddress_(baseAddress)
, memoryPool_(pool)
{
}

//MemoryBlockAllocator::MemoryBlockAllocator(
//    byte_t * baseAddress, 
//    size_t blockSize, 
//    size_t offsetWithinBlock,
//    MemoryBlockPool & container, 
//    size_t bufferSize)
//: baseAddress_(baseAddress)
//, memoryPool_(container)
//{
//    container.blockSize_ = blockSize;
//    memoryPool_.bufferSize_ = bufferSize;
//    memoryPool_.preAllocate(baseAddress_, offsetWithinBlock);
//}

bool MemoryBlockAllocator::allocate(Buffer & buffer, const Buffer::MemoryOwnerPtr & owner)
{
    return memoryPool_.allocate(baseAddress_, buffer, owner);
}

void MemoryBlockAllocator::release(Buffer & buffer)
{
    memoryPool_.release(baseAddress_, buffer);
}

size_t MemoryBlockAllocator::getBufferCapacity()const
{
    return memoryPool_.bufferSize_;
}

size_t MemoryBlockAllocator::getStorageSize()const
{
    return memoryPool_.blockSize_;
}

byte_t const * MemoryBlockAllocator::getStorageAddress()const
{
    return baseAddress_;
}

MemoryBlockPool & MemoryBlockAllocator::getContainer()
{
    return memoryPool_;
}

const MemoryBlockPool & MemoryBlockAllocator::getContainer() const
{
    return memoryPool_;
}

size_t MemoryBlockAllocator::getBufferCount()const
{
    return memoryPool_.bufferCount_;
}

bool MemoryBlockAllocator::hasMemoryAvailable() const
{
    return memoryPool_.rootOffset_ + memoryPool_.bufferSize_ <= memoryPool_.blockSize_;
}

