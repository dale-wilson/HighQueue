/// @file MemoryPool.cpp
#include <Common/HighQueuePch.h>
#include "MemoryPool.h"
#include <HighQueue/details/HQAllocator.h>

using namespace HighQueue;

MemoryPool::MemoryPool(size_t blockSize, size_t count)
    : allocatedSize_(HQMemoryBlockPool::spaceNeeded(blockSize, count) + CacheLineSize)
    , memory_(new byte_t[allocatedSize_])
    , pool_(*new (memory_.get()) HQMemoryBlockPool(allocatedSize_, blockSize))
    , numberOfAllocations_(0)
{
}

MemoryPool::~MemoryPool()
{
}

bool MemoryPool::tryAllocate(Message & message)
{
    if(pool_.tryAllocate(message))
    {
        ++numberOfAllocations_;
        return true;
    }
    return false;
}

void MemoryPool::allocate(Message & message)
{
    pool_.allocate(message);
    ++numberOfAllocations_;
}

size_t MemoryPool::getBlockCapacity()const
{
    return pool_.getBlockCapacity();
}

size_t MemoryPool::numberOfAllocations()const
{
    return numberOfAllocations_;
}

