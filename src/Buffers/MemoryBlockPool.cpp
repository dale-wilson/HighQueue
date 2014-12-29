#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockPool.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockPool::MemoryBlockPool()
: blockSize_(0)
, bufferSize_(0)
, bufferCount_(0)
, rootOffset_(0)
{
}

MemoryBlockPool::MemoryBlockPool(
    byte_t * baseAddress,
    size_t blockSize, 
    size_t bufferSize,
    size_t initialOffset)
: blockSize_(blockSize)
, bufferSize_(bufferSize)
, bufferCount_(0)
, rootOffset_(0)
{
    preAllocate(baseAddress, initialOffset);
}

size_t MemoryBlockPool::preAllocate(byte_t * baseAddress, size_t initialOffset)
{
    size_t offset = initialOffset + uintptr_t(baseAddress) % CacheLineSize;
    rootOffset_ = offset;
    bufferCount_ = 0;

    auto next = offset + bufferSize_;
    while(next <= blockSize_)
    {
        reinterpret_cast<size_t &>(baseAddress[offset]) = next;
        offset = next;
        next = offset + bufferSize_;
        bufferCount_ += 1;
    }
    return bufferCount_;
}

bool MemoryBlockPool::allocate(byte_t * baseAddress, Buffer & buffer, const Buffer::MemoryOwnerPtr & owner)
{
    Spinlock::Guard guard(lock_);
    bool ok = false;
    auto next = rootOffset_;
    if(next + bufferSize_ <= blockSize_)
    {
        rootOffset_ = reinterpret_cast<size_t &>(baseAddress[next]);
        buffer.set(owner, baseAddress, bufferSize_, next);
        ok = true;
    }
    return ok;
}

void MemoryBlockPool::release(byte_t * baseAddress, Buffer & buffer)
{
    if(buffer.getContainer() != baseAddress)
    {
        throw std::runtime_error("Buffer returned to wrong allocator.");
    }

    Spinlock::Guard guard(lock_);
    *reinterpret_cast<size_t *>(buffer.getContainer()) = rootOffset_;
    rootOffset_ = buffer.getOffset();
    buffer.reset();
}


size_t MemoryBlockPool::cacheAlignedBufferSize(size_t bufferSize)
{
    return ((bufferSize + CacheLineSize - 1) / CacheLineSize) * CacheLineSize;
}

size_t MemoryBlockPool::spaceNeeded(size_t bufferSize, size_t bufferCount)
{
    return cacheAlignedBufferSize(bufferSize) * bufferCount + CacheLineSize;
}

