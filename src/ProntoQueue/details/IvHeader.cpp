/// @file IvHeader.cpp
#include <Common/MPassPch.h>
#include "IvHeader.h"
#include <ProntoQueue/CreationParameters.h>
#include <ProntoQueue/details/IvAllocator.h>
#include <ProntoQueue/details/IvResolver.h>
#include <ProntoQueue/details/IvEntry.h>
#include <ProntoQueue/details/IvReservePosition.h>
#include <ProntoQueue/details/MemoryBlockPool.h>

using namespace MPass;
using namespace ProntoQueue;

IvHeader::IvHeader(
    const std::string & name,
    IvAllocator & allocator,
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
    reservePosition->reserveSoloPosition_ = entryCount_;

    auto messagePoolSize = ProntoQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    memoryPool_ = allocator.allocate(messagePoolSize, CacheLineSize);
    auto pool = new (resolver.resolve<ProntoQueue::MemoryBlockPool>(memoryPool_)) 
        ProntoQueue::MemoryBlockPool(messagePoolSize, parameters.messageSize_);

    allocateInternalMessages();

    signature_ = LiveSignature;
}

void IvHeader::allocateInternalMessages()
{
    IvResolver resolver(this);
    auto pool = resolver.resolve<ProntoQueue::MemoryBlockPool>(memoryPool_);
    auto entryPointer = resolver.resolve<IvEntry>(entries_);

    for(size_t nEntry = 0; nEntry < entryCount_; ++nEntry)
    {
        IvEntry & entry = entryPointer[nEntry];
        new (&entry) IvEntry;
        ProntoQueue::Message & message = entry.message_;
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
        ProntoQueue::Message & message = entry.message_;
        message.release();
    }
}

