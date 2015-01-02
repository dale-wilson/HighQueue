/// @file PQHeader.cpp
#include <Common/ProntoQueuePch.h>
#include "PQHeader.h"
#include <ProntoQueue/CreationParameters.h>
#include <ProntoQueue/details/PQAllocator.h>
#include <ProntoQueue/details/PQResolver.h>
#include <ProntoQueue/details/PQEntry.h>
#include <ProntoQueue/details/PQReservePosition.h>
#include <ProntoQueue/details/MemoryBlockPool.h>

using namespace ProntoQueue;

PQHeader::PQHeader(
    const std::string & name,
    PQAllocator & allocator,
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

    PQResolver resolver(this);

    entries_ = allocator.allocate(PQEntry::alignedSize() * entryCount_, CacheLineSize);
    readPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto readPosition = resolver.resolve<Position>(readPosition_);
    *readPosition = entryCount_;

    publishPosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto publishPosition = resolver.resolve<Position>(publishPosition_);
    *publishPosition = entryCount_;

    reservePosition_ = allocator.allocate(CacheLineSize, CacheLineSize);
    auto reservePosition = resolver.resolve<PQReservePosition>(reservePosition_);
    reservePosition->reservePosition_ = entryCount_;
    reservePosition->reserveSoloPosition_ = entryCount_;

    auto messagePoolSize = ProntoQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    memoryPool_ = allocator.allocate(messagePoolSize, CacheLineSize);
    auto pool = new (resolver.resolve<ProntoQueue::MemoryBlockPool>(memoryPool_)) 
        ProntoQueue::MemoryBlockPool(messagePoolSize, parameters.messageSize_);

    allocateInternalMessages();

    signature_ = LiveSignature;
}

void PQHeader::allocateInternalMessages()
{
    PQResolver resolver(this);
    auto pool = resolver.resolve<ProntoQueue::MemoryBlockPool>(memoryPool_);
    auto entryPointer = resolver.resolve<PQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        PQEntry & entry = entryPointer[nEntry];
        new (&entry) PQEntry;
        ProntoQueue::Message & message = entry.message_;
        if(!pool->allocate(message))
        {
            throw std::runtime_error("Not enough messages for entries.");
        }
    }
}

void PQHeader::releaseInternalMessages()
{
    PQResolver resolver(this);
    auto entryPointer = resolver.resolve<PQEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        PQEntry & entry = entryPointer[nEntry];
        ProntoQueue::Message & message = entry.message_;
        message.release();
    }
}

