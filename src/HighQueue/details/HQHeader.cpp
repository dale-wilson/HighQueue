// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/HighQueuePch.hpp>
#include "HQHeader.hpp"
#include <HighQueue/CreationParameters.hpp>
#include <HighQueue/details/HQAllocator.hpp>
#include <HighQueue/details/HQResolver.hpp>
#include <HighQueue/details/HQEntry.hpp>
#include <HighQueue/details/HQReservePosition.hpp>
#include <HighQueue/details/HQMemoryBlockPool.hpp>

using namespace HighQueue;

HQHeader::HQHeader(
    const std::string & name,
    HQAllocator & allocator,
    const CreationParameters & parameters,
    HQMemoryBlockPool * pool)
: signature_(InitializingSignature)
, version_(Version)
, discardMessagesIfNoConsumer_(parameters.discardMessagesIfNoConsumer_)
, producerWaitStrategy_(parameters.producerWaitStrategy_)
, consumerWaitStrategy_(parameters.consumerWaitStrategy_)
, entryCount_(parameters.entryCount_)
, entries_(0)
, readPosition_(0)
, publishPosition_(0)
, reservePosition_(0)
, memoryPool_(0)
, consumerPresent_(false)
, producersPresent_(0)
, waitMutex_()
, producerWaitConditionVariable_()
, consumerWaitConditionVariable_()
, producerWaiting_(false)
, consumerWaiting_(false)
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
    auto readPosition = resolver.resolve<AtomicPosition>(readPosition_);
    *readPosition = entryCount_;

    publishPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto publishPosition = resolver.resolve<AtomicPosition>(publishPosition_);
    *publishPosition = entryCount_;

    reservePosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto reservePosition = resolver.resolve<HighQReservePosition>(reservePosition_);
    new (reservePosition) HighQReservePosition(entryCount_);
//    reservePosition->reservePosition_ = entryCount_;
    if(pool == 0)
    {
        auto messagePoolSize = HQMemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
        memoryPool_ = allocator.allocate(messagePoolSize, CacheLineSize);
        pool = new (resolver.resolve<HQMemoryBlockPool>(memoryPool_))
            HQMemoryBlockPool(messagePoolSize, parameters.messageSize_);
    }

    allocateInternalMessages(pool);

    signature_ = LiveSignature;
}

void HQHeader::allocateInternalMessages(HQMemoryBlockPool * pool)
{
    HighQResolver resolver(this);
    auto entryPointer = resolver.resolve<HighQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        HighQEntry & entry = entryPointer[nEntry];
        new (&entry) HighQEntry(pool);
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

