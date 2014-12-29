#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockPool.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockPool::MemoryBlockPool()
: blockSize_(0)
, bufferSize_(0)
, bufferCount_(0)
, rootOffset_(NULL_OFFSET)
{
}

MemoryBlockPool::MemoryBlockPool(
    byte_t * baseAddress,
    size_t blockSize, 
    size_t bufferSize,
    size_t initialOffset)
: blockSize_(blockSize)
, bufferSize_(cacheAlignedBufferSize(bufferSize))
, bufferCount_(0)
, rootOffset_(NULL_OFFSET)
{
    preAllocate(baseAddress, initialOffset, bufferSize_, blockSize_);
}

size_t MemoryBlockPool::preAllocate(byte_t * baseAddress, size_t initialOffset, size_t bufferSize, size_t blockSize)
{
    size_t current = initialOffset + uintptr_t(baseAddress) % CacheLineSize;
    if(current + bufferSize_ > blockSize)
    {
        throw std::invalid_argument("MemoryBlockPool: aligned offset + buffer size exceeds pool size."); 
    }
    blockSize_ = blockSize;
    bufferSize_ = bufferSize;
    bufferCount_ = 0;

    rootOffset_ = current;
    while(current != NULL_OFFSET)
    {
        auto next = current + bufferSize_;
        if(next + bufferSize_ > blockSize_)
        {
            next = NULL_OFFSET;
        }
        reinterpret_cast<size_t &>(baseAddress[current]) = next;
        current = next;
        bufferCount_ += 1;
    }
//    std::cerr << "Preallocate " << bufferCount_ << " Buffers." << std::endl;
    return bufferCount_;
}

bool MemoryBlockPool::allocate(byte_t * baseAddress, Buffer & buffer, const Buffer::MemoryOwnerPtr & owner)
{
    Spinlock::Guard guard(lock_);
    auto offset = rootOffset_;
    auto ok = offset != NULL_OFFSET;
    if(ok)
    {
//        std::cerr << "Allocate buffer " << offset << std::endl;
        rootOffset_ = reinterpret_cast<size_t &>(baseAddress[offset]);
        buffer.set(owner, baseAddress, bufferSize_, offset);
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
//    std::cerr << "Release buffer " << buffer.getOffset() << std::endl;
    *buffer.get<size_t>() = rootOffset_;
    rootOffset_ = buffer.getOffset();
    buffer.reset();
}

bool MemoryBlockPool::isEmpty() const
{
    return rootOffset_ == NULL_OFFSET;
}


size_t MemoryBlockPool::cacheAlignedBufferSize(size_t bufferSize)
{
    return ((bufferSize + CacheLineSize - 1) / CacheLineSize) * CacheLineSize;
}

size_t MemoryBlockPool::spaceNeeded(size_t bufferSize, size_t bufferCount)
{
    return cacheAlignedBufferSize(bufferSize) * bufferCount + CacheLineSize;
}

