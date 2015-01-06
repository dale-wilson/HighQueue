/// @file HSQHeader.cpp
#include <Common/HSQueuePch.h>
#include "HSQHeader.h"
#include <HSQueue/CreationParameters.h>
#include <HSQueue/details/HSQAllocator.h>
#include <HSQueue/details/HSQResolver.h>
#include <HSQueue/details/HSQEntry.h>
#include <HSQueue/details/HSQReservePosition.h>
#include <HSQueue/details/MemoryBlockPool.h>

using namespace HSQueue;

HSQHeader::HSQHeader(
    const std::string & name,
    HSQAllocator & allocator,
    const CreationParameters & parameters)
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

    HSQResolver resolver(this);

    entries_ = allocator.allocate(HSQEntry::alignedSize() * entryCount_, CacheLineSize);
    readPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto readPosition = resolver.resolve<Position>(readPosition_);
    *readPosition = entryCount_;

    publishPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto publishPosition = resolver.resolve<Position>(publishPosition_);
    *publishPosition = entryCount_;

    reservePosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto reservePosition = resolver.resolve<HSQReservePosition>(reservePosition_);
    reservePosition->reservePosition_ = entryCount_;
    reservePosition->reserveSoloPosition_ = entryCount_;

    auto messagePoolSize = HSQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    memoryPool_ = allocator.allocate(messagePoolSize, CacheLineSize);
    auto pool = new (resolver.resolve<HSQueue::MemoryBlockPool>(memoryPool_)) 
        HSQueue::MemoryBlockPool(messagePoolSize, parameters.messageSize_);

    allocateInternalMessages();

    signature_ = LiveSignature;
}

void HSQHeader::allocateInternalMessages()
{
    HSQResolver resolver(this);
    auto pool = resolver.resolve<HSQueue::MemoryBlockPool>(memoryPool_);
    auto entryPointer = resolver.resolve<HSQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        HSQEntry & entry = entryPointer[nEntry];
        new (&entry) HSQEntry;
        HSQueue::Message & message = entry.message_;
        if(!pool->allocate(message))
        {
            throw std::runtime_error("Not enough messages for entries.");
        }
    }
}

void HSQHeader::releaseInternalMessages()
{
    HSQResolver resolver(this);
    auto entryPointer = resolver.resolve<HSQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        HSQEntry & entry = entryPointer[nEntry];
        HSQueue::Message & message = entry.message_;
        message.release();
    }
}

