#include <Common/MPassPch.h>

#include <Buffers/MemoryBlockInfo.h>

using namespace MPass;
using namespace Buffers;

MemoryBlockInfo::MemoryBlockInfo()
: blockSize_(0)
, bufferSize_(0)
, bufferCount_(0)
, rootOffset_(0)
{
}

MemoryBlockInfo::MemoryBlockInfo(
    size_t blockSize, 
    size_t bufferSize)
: blockSize_(blockSize)
, bufferSize_(bufferSize)
, bufferCount_(0)
, rootOffset_(0)
{
}

size_t MemoryBlockInfo::preAllocate(byte_t * baseAddress, size_t initialOffset)
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

bool MemoryBlockInfo::allocate(byte_t * baseAddress, Buffer & buffer, const Buffer::MemoryOwnerPtr & owner)
{
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

void MemoryBlockInfo::release(byte_t * baseAddress, Buffer & buffer)
{
    if(buffer.getContainer() != baseAddress)
    {
        throw std::runtime_error("Buffer returned to wrong allocator.");
    }

    *reinterpret_cast<size_t *>(buffer.getContainer()) = rootOffset_;
    rootOffset_ = buffer.getOffset();
    buffer.reset();
}
