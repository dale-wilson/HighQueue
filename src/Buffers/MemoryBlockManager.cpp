#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockManager.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockManager::MemoryBlockManager(byte_t * baseAddress, Buffers::MemoryBlockPool & blockInfo)
    : allocator_(baseAddress, blockInfo)
{
}

bool MemoryBlockManager::allocate(Buffer & buffer)
{
    return allocator_.allocate(buffer, shared_from_this());
}

size_t MemoryBlockManager::getBufferCapacity()const
{
    return allocator_.getBufferCapacity();
}

size_t MemoryBlockManager::getStorageSize()const
{
    return allocator_.getStorageSize();
}

byte_t const * MemoryBlockManager::getStorageAddress()const
{
    return allocator_.getStorageAddress();
}

size_t MemoryBlockManager::getBufferCount()const
{
    return allocator_.getBufferCount();
}

bool MemoryBlockManager::hasMemoryAvailable() const
{
    return allocator_.hasMemoryAvailable();
}

void MemoryBlockManager::release(Buffer & buffer)
{
    allocator_.release(buffer);
}
