#include <Common/MPassPch.h>

#include <Buffers/BufferAllocator.h>

using namespace MPass;
using namespace Buffers;

size_t BufferAllocator::cacheAlignedBufferSize(size_t bufferSize)
{
    return ((bufferSize + CacheLineSize - 1) / CacheLineSize) * CacheLineSize;
}

size_t BufferAllocator::spaceNeeded(size_t bufferSize, size_t bufferCount)
{
    return cacheAlignedBufferSize(bufferSize) * bufferCount + CacheLineSize;
}

BufferAllocator::BufferAllocator(size_t bufferSize, size_t bufferCount)
: block_(new byte_t[spaceNeeded(bufferSize, bufferCount)])
, baseAddress_(block_.get())
, internalContainer_(spaceNeeded(bufferSize, bufferCount), cacheAlignedBufferSize(bufferSize))
, container_(internalContainer_)
{
   container_.preAllocate(baseAddress_, 0);
}

BufferAllocator::BufferAllocator(size_t bufferSize, byte_t * block, size_t blockSize)
: block_()
, baseAddress_(block)
, internalContainer_(blockSize, cacheAlignedBufferSize(bufferSize))
, container_(internalContainer_)
{
   container_.preAllocate(baseAddress_, 0);
}

BufferAllocator::BufferAllocator(byte_t * baseAddress, BufferContainer & container)
: block_()
, baseAddress_(baseAddress)
, internalContainer_()
, container_(container)
{
}

BufferAllocator::BufferAllocator(
    byte_t * block, 
    size_t blockSize, 
    size_t offsetWithinBlock,
    BufferContainer & container, 
    size_t bufferSize)
: block_()
, baseAddress_(block)
, internalContainer_()
, container_(container)
{
    container.blockSize_ = blockSize;
    container_.bufferSize_ = bufferSize;
    container_.preAllocate(baseAddress_, offsetWithinBlock);
}



bool BufferAllocator::allocate(Buffer & buffer)
{
    return container_.allocate(baseAddress_, buffer);
}

void BufferAllocator::free(Buffer & buffer)
{
    container_.free(baseAddress_, buffer);
}


BufferAllocator::BufferContainer::BufferContainer()
: blockSize_(0)
, bufferSize_(0)
, bufferCount_(0)
, rootOffset_(0)
{
}

BufferAllocator::BufferContainer::BufferContainer(
    size_t blockSize, 
    size_t bufferSize)
: blockSize_(blockSize)
, bufferSize_(bufferSize)
, bufferCount_(0)
, rootOffset_(0)
{
}

size_t BufferAllocator::BufferContainer::preAllocate(byte_t * baseAddress, size_t initialOffset)
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

bool BufferAllocator::BufferContainer::allocate(byte_t * baseAddress, Buffer & buffer)
{
    bool ok = false;
    auto next = rootOffset_;
    if(next + bufferSize_ <= blockSize_)
    {
        rootOffset_ = reinterpret_cast<size_t &>(baseAddress[next]);
        buffer.set(baseAddress, bufferSize_, next);
        ok = true;
    }
    return ok;
}

void BufferAllocator::BufferContainer::free(byte_t * baseAddress, Buffer & buffer)
{
    if(buffer.getContainer() != baseAddress)
    {
        throw std::runtime_error("Buffer returned to wrong allocator.");
    }

    *reinterpret_cast<size_t *>(buffer.getContainer()) = rootOffset_;
    rootOffset_ = buffer.getOffset();
    buffer.forget();
}

