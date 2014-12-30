/// @file IvHeader.cpp
#include <Common/MPassPch.h>
#include "IvHeader.h"
#include <InfiniteVector/IvCreationParameters.h>
#include <InfiniteVector/IvAllocator.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvEntry.h>
#include <InfiniteVector/IvReservePosition.h>
#include <InfiniteVector/MemoryBlockPool.h>

using namespace MPass;
using namespace InfiniteVector;

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
, consumerWaitStrategy_(parameters.strategy_)
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

    auto messagePoolSize = InfiniteVector::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    memoryPool_ = allocator.allocate(messagePoolSize, CacheLineSize);
    auto pool = new (resolver.resolve<InfiniteVector::MemoryBlockPool>(memoryPool_)) 
        InfiniteVector::MemoryBlockPool(messagePoolSize, parameters.messageSize_);

    allocateInternalMessages();

    signature_ = LiveSignature;
}

void IvHeader::allocateInternalMessages()
{
    IvResolver resolver(this);
    auto pool = resolver.resolve<InfiniteVector::MemoryBlockPool>(memoryPool_);
    auto entryPointer = resolver.resolve<IvEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        IvEntry & entry = entryPointer[nEntry];
        new (&entry) IvEntry;
        InfiniteVector::Message & message = entry.message_;
        if(!pool->allocate(message))
        {
            throw std::runtime_error("Not enough messages for entries.");
        }
    }
}

void IvHeader::releaseInternalMessages()
{
    IvResolver resolver(this);
    auto entryPointer = resolver.resolve<IvEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        IvEntry & entry = entryPointer[nEntry];
        InfiniteVector::Message & message = entry.message_;
        message.release();
    }
}

