#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockHolder.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockHolder::MemoryBlockHolder(size_t bufferSize, size_t bufferCount)
    : blockSize_(MemoryBlockPool::spaceNeeded(bufferSize, bufferCount))
    , baseAddress_(new byte_t[blockSize_])
    , internalPool_(baseAddress_.get(), blockSize_, bufferSize)
    , allocator_(new MemoryBlockAllocator(baseAddress_.get(), internalPool_))
{
}

bool MemoryBlockHolder::allocate(Buffer & buffer)
{
    Buffer::MemoryOwnerPtr owner(shared_from_this());
    return allocator_->allocate(buffer, owner);
}

size_t MemoryBlockHolder::getBufferCapacity()const
{
    return allocator_->getBufferCapacity();
}

size_t MemoryBlockHolder::getStorageSize()const
{
    return allocator_->getStorageSize();
}

byte_t const * MemoryBlockHolder::getStorageAddress()const
{
    return allocator_->getStorageAddress();
}

size_t MemoryBlockHolder::getBufferCount()const
{
    return allocator_->getBufferCount();
}

bool MemoryBlockHolder::isEmpty() const
{
    return allocator_->isEmpty();
}

void MemoryBlockHolder::release(Buffer & buffer)
{
    allocator_->release(buffer);
}
