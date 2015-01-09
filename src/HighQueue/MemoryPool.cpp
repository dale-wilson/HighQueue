/// @file MemoryPool.cpp
#include <Common/HighQueuePch.h>
#include "MemoryPool.h"
#include <HighQueue/details/HQAllocator.h>

using namespace HighQueue;

MemoryPool::MemoryPool(size_t blockSize, size_t count)
    : allocatedSize_(HQMemoryBLockPool::spaceNeeded(blockSize, count) + CacheLineSize)
    , memory_(new byte_t[allocatedSize_])
    , pool_(*new (memory_.get()) HQMemoryBLockPool(allocatedSize_, blockSize))
{
}

MemoryPool::~MemoryPool()
{
}

bool MemoryPool::tryAllocate(Message & message)
{
    return pool_.tryAllocate(message);
}

void MemoryPool::allocate(Message & message)
{
    pool_.allocate(message);
}

size_t MemoryPool::getBlockCapacity()const
{
    return pool_.getBlockCapacity();
}
