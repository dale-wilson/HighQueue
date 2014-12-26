/// @file IvHeader.cpp
#include <Common/MPassPch.h>
#include "IvHeader.h"
#include <InfiniteVector/IvCreationParameters.h>
#include <InfiniteVector/IvAllocator.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvEntry.h>
#include <InfiniteVector/IvReservePosition.h>
#include <Buffers/MemoryBlockAllocator.h>

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
    , readPosition_(0)
    , publishPosition_(0)
    , reservePosition_(0)
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
    readPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto readPosition = resolver.resolve<Position>(readPosition_);
    *readPosition = entryCount_;

    publishPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto publishPosition = resolver.resolve<Position>(publishPosition_);
    *publishPosition = entryCount_;

    reservePosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto reservePosition = resolver.resolve<IvReservePosition>(reservePosition_);
    reservePosition->reservePosition_ = entryCount_;
    new (&reservePosition->spinlock_) Spinlock();   

    auto bufferBase = reinterpret_cast<byte_t *>(this);
    auto cacheAlignedBufferSize = Buffers::MemoryBlockAllocator::cacheAlignedBufferSize(parameters.bufferSize_);
    auto blockSize = cacheAlignedBufferSize * parameters.bufferCount_;
    auto blockOffset = allocator.allocate(blockSize, CacheLineSize);
    blockInfo_ = Buffers::MemoryBlockInfo(
        blockOffset + blockSize, cacheAlignedBufferSize);
    blockInfo_.preAllocate(bufferBase, blockOffset);

    // Now initialize the entries that were previously allocated.
    auto entryPointer = resolver.resolve<IvEntry>(entries_);
    for(size_t nEntry = 0; nEntry < parameters.entryCount_; ++nEntry)
    {
        // consider an entry resolver if the alignment doesn't work out.
        IvEntry & entry = entryPointer[nEntry];
        new (&entry) IvEntry;
        Buffers::Buffer & buffer = entry.buffer_;
        if(!blockInfo_.allocate(bufferBase, buffer))
        {
            throw std::runtime_error("Not enough buffers for entries.");
        }
    }

    signature_ = LiveSignature;
 }

} // InfiniteVector
} // MPass