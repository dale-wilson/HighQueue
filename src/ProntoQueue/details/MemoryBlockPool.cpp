#include <Common/MPassPch.h>

#include <ProntoQueue/details/MemoryBlockPool.h>

using namespace MPass;
using namespace ProntoQueue;

MemoryBlockPool::MemoryBlockPool()
: blockSize_(0)
, messageSize_(0)
, messageCount_(0)
, rootOffset_(NULL_OFFSET)
{
}

MemoryBlockPool::MemoryBlockPool(
    size_t blockSize, 
    size_t messageSize)
: blockSize_(blockSize)
, messageSize_(cacheAlignedMessageSize(messageSize))
, messageCount_(0)
, rootOffset_(NULL_OFFSET)
{
    preAllocate(messageSize_, blockSize_);
}

size_t MemoryBlockPool::preAllocate(size_t messageSize, size_t blockSize)
{
    auto sizeofthis = sizeof(MemoryBlockPool);
    auto intThis = uintptr_t(this);
    auto endThis = intThis + sizeofthis + CacheLineSize - 1;
    endThis -= endThis % CacheLineSize;
    size_t current = endThis - intThis;
    if(current + messageSize_ > blockSize)
    {
        throw std::invalid_argument("MemoryBlockPool: aligned offset + message size exceeds pool size."); 
    }
    blockSize_ = blockSize;
    messageSize_ = messageSize;
    messageCount_ = 0;

    auto baseAddress = reinterpret_cast<byte_t *>(this);
    rootOffset_ = current;
    while(current != NULL_OFFSET)
    {
//        std::cerr << "Create " << current << std::endl;
        auto next = current + messageSize_;
        if(next + messageSize_ > blockSize_)
        {
            next = NULL_OFFSET;
        }

        reinterpret_cast<size_t &>(baseAddress[current]) = next;
        current = next;
        messageCount_ += 1;
    }
//    std::cerr << "Preallocate " << messageCount_ << " Messages. " << rootOffset_ << std::endl;
    return messageCount_;
}

bool MemoryBlockPool::allocate(Message & message)
{
    Spinlock::Guard guard(lock_);
    auto offset = rootOffset_;
    auto ok = offset != NULL_OFFSET;
    if(ok)
    {
//        std::cout << "Allocate " << offset << std::endl;
        auto baseAddress = reinterpret_cast<byte_t *>(this);
        rootOffset_ = reinterpret_cast<size_t &>(baseAddress[offset]);
        message.set(this, messageSize_, offset, 0);
    }
    return ok;
}

void MemoryBlockPool::release(Message & message)
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

bool MemoryBlockPool::isEmpty() const
{
    return rootOffset_ == NULL_OFFSET;
}


size_t MemoryBlockPool::cacheAlignedMessageSize(size_t messageSize)
{
    return ((messageSize + CacheLineSize - 1) / CacheLineSize) * CacheLineSize;
}

size_t MemoryBlockPool::spaceNeeded(size_t messageSize, size_t messageCount)
{
    return sizeof(MemoryBlockPool) + cacheAlignedMessageSize(messageSize) * messageCount + CacheLineSize;
}
