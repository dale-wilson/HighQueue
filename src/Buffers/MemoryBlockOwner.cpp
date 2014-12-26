#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockOwner.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockOwner::MemoryBlockOwner(size_t bufferSize, size_t bufferCount)
    : blockSize_(MemoryBlockAllocator::spaceNeeded(bufferSize, bufferCount))
    , block_(new byte_t[blockSize_])
    , allocator_(block_.get(), blockSize_, bufferSize)
{
}

bool MemoryBlockOwner::allocate(Buffer & buffer)
{
    return allocator_.allocate(buffer);
}

size_t MemoryBlockOwner::getBufferSize()const
{
    return allocator_.getBufferSize();
}

size_t MemoryBlockOwner::getStorageSize()const
{
    return allocator_.getStorageSize();
}

byte_t const * MemoryBlockOwner::getStorageAddress()const
{
    return allocator_.getStorageAddress();
}

size_t MemoryBlockOwner::getBufferCount()const
{
    return allocator_.getBufferCount();
}

bool MemoryBlockOwner::hasBuffers() const
{
    return allocator_.hasBuffers();
}

void MemoryBlockOwner::release(Buffer & buffer)
{
    allocator_.free(buffer);
}
