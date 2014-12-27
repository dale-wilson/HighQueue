#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockAllocator.h>

using namespace MPass;
using namespace Buffers;

size_t MemoryBlockAllocator::cacheAlignedBufferSize(size_t bufferSize)
{
    return ((bufferSize + CacheLineSize - 1) / CacheLineSize) * CacheLineSize;
}

size_t MemoryBlockAllocator::spaceNeeded(size_t bufferSize, size_t bufferCount)
{
    return cacheAlignedBufferSize(bufferSize) * bufferCount + CacheLineSize;
}


MemoryBlockAllocator::MemoryBlockAllocator(byte_t * baseAddress, size_t blockSize, size_t bufferSize)
: baseAddress_(baseAddress)
, internalContainer_(blockSize, cacheAlignedBufferSize(bufferSize))
, container_(internalContainer_)
{
   container_.preAllocate(baseAddress_, 0);
}

MemoryBlockAllocator::MemoryBlockAllocator(byte_t * baseAddress, MemoryBlockInfo & container)
: baseAddress_(baseAddress)
, internalContainer_()
, container_(container)
{
}

MemoryBlockAllocator::MemoryBlockAllocator(
    byte_t * block, 
    size_t blockSize, 
    size_t offsetWithinBlock,
    MemoryBlockInfo & container, 
    size_t bufferSize)
: baseAddress_(block)
, internalContainer_()
, container_(container)
{
    container.blockSize_ = blockSize;
    container_.bufferSize_ = bufferSize;
    container_.preAllocate(baseAddress_, offsetWithinBlock);
}

bool MemoryBlockAllocator::allocate(Buffer & buffer)
{
    return container_.allocate(baseAddress_, buffer);
}

void MemoryBlockAllocator::free(Buffer & buffer)
{
    container_.free(baseAddress_, buffer);
}

size_t MemoryBlockAllocator::getBufferCapacity()const
{
    return container_.bufferSize_;
}

size_t MemoryBlockAllocator::getStorageSize()const
{
    return container_.blockSize_;
}

byte_t const * MemoryBlockAllocator::getStorageAddress()const
{
    return baseAddress_;
}

MemoryBlockInfo & MemoryBlockAllocator::getContainer()
{
    return container_;
}

const MemoryBlockInfo & MemoryBlockAllocator::getContainer() const
{
    return container_;
}

size_t MemoryBlockAllocator::getBufferCount()const
{
    return container_.bufferCount_;
}

bool MemoryBlockAllocator::hasMemoryAvailable() const
{
    return container_.rootOffset_ + container_.bufferSize_ <= container_.blockSize_;
}

