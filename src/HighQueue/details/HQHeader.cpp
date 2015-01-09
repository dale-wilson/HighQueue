/// @file HQHeader.cpp
#include <Common/HighQueuePch.h>
#include "HQHeader.h"
#include <HighQueue/CreationParameters.h>
#include <HighQueue/details/HQAllocator.h>
#include <HighQueue/details/HQResolver.h>
#include <HighQueue/details/HQEntry.h>
#include <HighQueue/details/HQReservePosition.h>
#include <HighQueue/details/HQMemoryBLockPool.h>

using namespace HighQueue;

HQHeader::HQHeader(
    const std::string & name,
    HQAllocator & allocator,
    const CreationParameters & parameters,
    HQMemoryBLockPool * pool)
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
    std::memset(name_, '\0', sizeof(name_));
    size_t bytesToCopy = name.size();
    if(bytesToCopy >= sizeof(name_) - 1)
    {
        bytesToCopy = sizeof(name_) - 1;
    }
    std::memcpy(name_, name.data(), bytesToCopy);

    HighQResolver resolver(this);

    entries_ = allocator.allocate(HighQEntry::alignedSize() * entryCount_, CacheLineSize);
    readPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto readPosition = resolver.resolve<Position>(readPosition_);
    *readPosition = entryCount_;

    publishPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto publishPosition = resolver.resolve<Position>(publishPosition_);
    *publishPosition = entryCount_;

    reservePosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto reservePosition = resolver.resolve<HighQReservePosition>(reservePosition_);
    reservePosition->reservePosition_ = entryCount_;
    reservePosition->reserveSoloPosition_ = entryCount_;
    if(pool == 0)
    {
        auto messagePoolSize = HQMemoryBLockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
        memoryPool_ = allocator.allocate(messagePoolSize, CacheLineSize);
        pool = new (resolver.resolve<HQMemoryBLockPool>(memoryPool_))
            HQMemoryBLockPool(messagePoolSize, parameters.messageSize_);
    }

    allocateInternalMessages(pool);

    signature_ = LiveSignature;
}

void HQHeader::allocateInternalMessages(HQMemoryBLockPool * pool)
{
    HighQResolver resolver(this);
    auto entryPointer = resolver.resolve<HighQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        HighQEntry & entry = entryPointer[nEntry];
        new (&entry) HighQEntry(*pool);
    }
}

void HQHeader::releaseInternalMessages()
{
    HighQResolver resolver(this);
    auto entryPointer = resolver.resolve<HighQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        HighQEntry & entry = entryPointer[nEntry];
        Message & message = entry.message_;
        message.release();
    }
}

