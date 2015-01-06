/// @file Connection.cpp
#include <Common/HSQueuePch.h>
#include "Connection.h"
#include <HSQueue/details/HSQAllocator.h>
#include <HSQueue/details/HSQEntry.h>
#include <HSQueue/details/HSQResolver.h>

using namespace HSQueue;

Connection::Connection()
{
}

Connection::~Connection()
{
    if(localMemory_ && header_)
    {
        header_->releaseInternalMessages();
    }
}

void Connection::close()
{
    if(localMemory_ && header_)
    {
        header_->releaseInternalMessages();
    }
}


void Connection::createLocal(const std::string & name, const CreationParameters & parameters)
{
    const size_t allocatedSize = spaceNeeded(parameters);
    localMemory_.reset(new byte_t[allocatedSize]);
    try
    {
        byte_t * block = localMemory_.get();
        byte_t * alignedBlock = HSQAllocator::align(block, CacheLineSize);
        size_t availableSize = allocatedSize - (alignedBlock - block);

        HSQAllocator allocator(availableSize, sizeof(HSQHeader));
        header_ = new (alignedBlock)HSQHeader(name, allocator, parameters);
        HSQResolver resolver(header_);
        memoryPool_ = resolver.resolve<HSQueue::MemoryBlockPool>(header_->memoryPool_);
    }
    catch(...)
    {
        localMemory_.reset();
        header_ = 0;
        memoryPool_ = 0;
        throw;
    }
}

HSQHeader * Connection::getHeader() const
{
    return header_;
}
            
void Connection::openOrCreateShared(const std::string & name, const CreationParameters & parameters)
{
// todo
}

void Connection::openExistingShared(const std::string & name)
{
// todo
}

size_t Connection::spaceNeeded(const CreationParameters & parameters)
{
    size_t headerSize = HSQAllocator::align(sizeof(HSQHeader), CacheLineSize);
    size_t entriesSize = HSQEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that positions fit in a single cache line
    size_t messagePoolSize = HSQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    size_t cacheAlignmentSize = CacheLineSize;
    return headerSize + entriesSize + positionsSize + messagePoolSize + cacheAlignmentSize;
}

bool Connection::allocate(HSQueue::Message & message)
{
    return memoryPool_->allocate(message);
}

size_t Connection::getMessageCapacity()const
{
    return memoryPool_->getMessageCapacity();
}
size_t Connection::getMessageCount()const
{
    return memoryPool_->getMessageCount();
}
bool Connection::hasMemoryAvailable() const
{
    return !memoryPool_->isEmpty();
}
