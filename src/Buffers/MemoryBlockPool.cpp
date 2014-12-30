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
    size_t blockSize, 
    size_t bufferSize)
: blockSize_(blockSize)
, bufferSize_(cacheAlignedBufferSize(bufferSize))
, bufferCount_(0)
, rootOffset_(NULL_OFFSET)
{
    preAllocate(bufferSize_, blockSize_);
}

size_t MemoryBlockPool::preAllocate(size_t bufferSize, size_t blockSize)
{
    auto sizeofthis = sizeof(MemoryBlockPool);
    auto intThis = uintptr_t(this);
    auto endThis = intThis + sizeofthis + CacheLineSize - 1;
    endThis -= endThis % CacheLineSize;
    size_t current = endThis - intThis;
    if(current + bufferSize_ > blockSize)
    {
        throw std::invalid_argument("MemoryBlockPool: aligned offset + buffer size exceeds pool size."); 
    }
    blockSize_ = blockSize;
    bufferSize_ = bufferSize;
    bufferCount_ = 0;

    auto baseAddress = reinterpret_cast<byte_t *>(this);
    rootOffset_ = current;
    while(current != NULL_OFFSET)
    {
//        std::cerr << "Create " << current << std::endl;
        auto next = current + bufferSize_;
        if(next + bufferSize_ > blockSize_)
        {
            next = NULL_OFFSET;
        }

        reinterpret_cast<size_t &>(baseAddress[current]) = next;
        current = next;
        bufferCount_ += 1;
    }
//    std::cerr << "Preallocate " << bufferCount_ << " Buffers. " << rootOffset_ << std::endl;
    return bufferCount_;
}

bool MemoryBlockPool::allocate(Buffer & buffer)
{
    Spinlock::Guard guard(lock_);
    auto offset = rootOffset_;
    auto ok = offset != NULL_OFFSET;
    if(ok)
    {
//        std::cout << "Allocate " << offset << std::endl;
        auto baseAddress = reinterpret_cast<byte_t *>(this);
        rootOffset_ = reinterpret_cast<size_t &>(baseAddress[offset]);
        buffer.set(this, bufferSize_, offset, 0);
    }
    return ok;
}

void MemoryBlockPool::release(Buffer & buffer)
{
    auto baseAddress = reinterpret_cast<byte_t *>(this);
    if(buffer.getContainer() != baseAddress)
    {
        throw std::runtime_error("Buffer returned to wrong allocator.");
    }

    Spinlock::Guard guard(lock_);
//    std::cout << "Release " << buffer.getOffset() << std::endl;
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
    return sizeof(MemoryBlockPool) + cacheAlignedBufferSize(bufferSize) * bufferCount + CacheLineSize;
}
