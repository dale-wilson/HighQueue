#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockHolder.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockHolder::MemoryBlockHolder(size_t bufferSize, size_t bufferCount)
    : blockSize_(MemoryBlockAllocator::spaceNeeded(bufferSize, bufferCount))
    , block_(new byte_t[blockSize_])
    , allocator_(block_.get(), blockSize_, bufferSize)
{
}

bool MemoryBlockHolder::allocate(Buffer & buffer)
{
    return allocator_.allocate(buffer);
}

size_t MemoryBlockHolder::getBufferCapacity()const
{
    return allocator_.getBufferCapacity();
}

size_t MemoryBlockHolder::getStorageSize()const
{
    return allocator_.getStorageSize();
}

byte_t const * MemoryBlockHolder::getStorageAddress()const
{
    return allocator_.getStorageAddress();
}

size_t MemoryBlockHolder::getBufferCount()const
{
    return allocator_.getBufferCount();
}

bool MemoryBlockHolder::hasMemoryAvailable() const
{
    return allocator_.hasMemoryAvailable();
}

void MemoryBlockHolder::release(Buffer & buffer)
{
    allocator_.release(buffer);
}
