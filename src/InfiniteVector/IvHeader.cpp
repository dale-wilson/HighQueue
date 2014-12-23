/// @file IvHeader.cpp
#include <Common/MPassPch.h>
#include "IvHeader.h"
#include <InfiniteVector/IvCreationParameters.h>
#include <InfiniteVector/IvAllocator.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvEntry.h>
#include <InfiniteVector/IvReservePositionStructure.h>
#include <Buffers/BufferAllocator.h>

namespace MPass
{
namespace InfiniteVector
{
    IvHeader::IvHeader(
        const std::string & name, 
        IvAllocator & allocator, 
        const IvCreationParameters & parameters)
    : signature_(InitializingSignature)
    , version_(Version)
    , entryCount_(parameters.entryCount_)
    , entries_(0)
    , positions_(0)
    , consumerWaitsViaMutexCondition_(false)
    , consumerWaitMutex_()
    , consumerWaitConditionVariable_()
{
    uintptr_t here = reinterpret_cast<uintptr_t>(this);
    if(IvAllocator::align(here, CacheLineSize) != here)
    {
        throw std::exception("IvHeader must be allocated on cache line boundary");
    }

    std::memset(name_, '\0', sizeof(name_));
    size_t bytesToCopy = name.size();
    if(bytesToCopy >= sizeof(name_) - 1)
    {
        bytesToCopy = sizeof(name_) - 1;
    }
    std::memcpy(name_, name.data(), bytesToCopy);

    IvResolver resolver(this);

    entries_ = allocator.allocate(IvEntry::alignedSize() * entryCount_, CacheLineSize);
    positions_ = allocator.allocate(3 * CacheLineSize, CacheLineSize);

    auto readPosition = resolver.resolve<Position>(positions_, CacheLineSize, 0);
    *readPosition = entryCount_;
    auto publishPosition = resolver.resolve<Position>(positions_, CacheLineSize, 1);
    *publishPosition = entryCount_;
    auto reservePosition = resolver.resolve<IvReservePositionStructure>(positions_, CacheLineSize, 2);
    reservePosition->reservePosition_ = entryCount_;
    new (&reservePosition->spinLock_) SpinLock();   

    auto bufferBase = reinterpret_cast<byte_t *>(this);
    auto cacheAlignedBufferSize = Buffers::BufferAllocator::cacheAlignedBufferSize(parameters.bufferSize_);
    auto blockSize = cacheAlignedBufferSize * parameters.bufferCount_;
    auto blockOffset = allocator.allocate(blockSize, CacheLineSize);
    bufferContainer_ = Buffers::BufferAllocator::BufferContainer(
        blockOffset + blockSize, cacheAlignedBufferSize);
    bufferContainer_.preAllocate(bufferBase, blockOffset);

    // Now initialize the entries that were previously allocated.
    auto entryPointer = resolver.resolve<IvEntry>(entries_);

    for(size_t nEntry = 0; nEntry < parameters.entryCount_; ++nEntry)
    {
        // consider an entry resolver if the alignment doesn't work out.
        IvEntry & entry = entryPointer[nEntry];
        new (&entry) IvEntry;
        Buffers::Buffer & buffer = entry.buffer_;
        if(!bufferContainer_.allocate(bufferBase, buffer))
        {
            throw std::runtime_error("Not enough buffers for entries.");
        }
    }

    signature_ = LiveSignature;
 }

} // InfiniteVector
} // MPass