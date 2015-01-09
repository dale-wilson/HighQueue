#include <Common/HighQueuePch.h>

#include <HighQueue/details/HQMemoryBLockPool.h>

using namespace HighQueue;

HQMemoryBLockPool::HQMemoryBLockPool()
: poolSize_(0)
, blockSize_(0)
, blockCount_(0)
, rootOffset_(NULL_OFFSET)
{
}

HQMemoryBLockPool::HQMemoryBLockPool(
    size_t poolSize, 
    size_t blockSize)
: poolSize_(poolSize)
, blockSize_(cacheAlignedMessageSize(blockSize))
, blockCount_(0)
, rootOffset_(NULL_OFFSET)
{
    preAllocate(blockSize_, poolSize_);
}

size_t HQMemoryBLockPool::preAllocate(size_t blockSize, size_t poolSize)
{
    auto sizeofthis = sizeof(HQMemoryBLockPool);
    auto intThis = uintptr_t(this);
    auto endThis = intThis + sizeofthis + CacheLineSize - 1;
    endThis -= endThis % CacheLineSize;
    size_t current = endThis - intThis;
    if(current + blockSize_ > poolSize)
    {
        throw std::invalid_argument("HQMemoryBLockPool: aligned offset + message size exceeds pool size."); 
    }
    poolSize_ = poolSize;
    blockSize_ = blockSize;
    blockCount_ = 0;

    auto baseAddress = reinterpret_cast<byte_t *>(this);
    rootOffset_ = current;
    while(current != NULL_OFFSET)
    {
//        std::cerr << "Create " << current << std::endl;
        auto next = current + blockSize_;
        if(next + blockSize_ > poolSize_)
        {
            next = NULL_OFFSET;
        }

        reinterpret_cast<size_t &>(baseAddress[current]) = next;
        current = next;
        blockCount_ += 1;
    }
//    std::cerr << "Preallocate " << blockCount_ << " Messages. " << rootOffset_ << std::endl;
    return blockCount_;
}

bool HQMemoryBLockPool::allocate(Message & message)
{
    Spinlock::Guard guard(lock_);
    auto offset = rootOffset_;
    auto ok = offset != NULL_OFFSET;
    if(ok)
    {
//        std::cout << "Allocate " << offset << std::endl;
        auto baseAddress = reinterpret_cast<byte_t *>(this);
        rootOffset_ = reinterpret_cast<size_t &>(baseAddress[offset]);
        message.set(this, blockSize_, offset, 0);
    }
    return ok;
}

void HQMemoryBLockPool::release(Message & message)
{
    auto baseAddress = reinterpret_cast<byte_t *>(this);
    if(message.getContainer() != baseAddress)
    {
        throw std::runtime_error("Message returned to wrong allocator.");
    }

    Spinlock::Guard guard(lock_);
//    std::cout << "Release " << message.getOffset() << std::endl;
    *message.get<size_t>() = rootOffset_;
    rootOffset_ = message.getOffset();
    message.reset();
}

bool HQMemoryBLockPool::isEmpty() const
{
    return rootOffset_ == NULL_OFFSET;
}


size_t HQMemoryBLockPool::cacheAlignedMessageSize(size_t blockSize)
{
    return ((blockSize + CacheLineSize - 1) / CacheLineSize) * CacheLineSize;
}

size_t HQMemoryBLockPool::spaceNeeded(size_t blockSize, size_t messageCount)
{
    return sizeof(HQMemoryBLockPool) + cacheAlignedMessageSize(blockSize) * messageCount + CacheLineSize;
}
