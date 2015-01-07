/// @file Connection.cpp
#include <Common/HighQueuePch.h>
#include "Connection.h"
#include <HighQueue/details/HQAllocator.h>
#include <HighQueue/details/HQEntry.h>
#include <HighQueue/details/HQResolver.h>

using namespace HighQueue;

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
        byte_t * alignedBlock = HighQAllocator::align(block, CacheLineSize);
        size_t availableSize = allocatedSize - (alignedBlock - block);

        HighQAllocator allocator(availableSize, sizeof(HQHeader));
        header_ = new (alignedBlock)HQHeader(name, allocator, parameters);
        HighQResolver resolver(header_);
        memoryPool_ = resolver.resolve<HighQueue::MemoryBlockPool>(header_->memoryPool_);
    }
    catch(...)
    {
        localMemory_.reset();
        header_ = 0;
        memoryPool_ = 0;
        throw;
    }
}

HQHeader * Connection::getHeader() const
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
    size_t headerSize = HighQAllocator::align(sizeof(HQHeader), CacheLineSize);
    size_t entriesSize = HighQEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that positions fit in a single cache line
    size_t messagePoolSize = HighQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    size_t cacheAlignmentSize = CacheLineSize;
    return headerSize + entriesSize + positionsSize + messagePoolSize + cacheAlignmentSize;
}

bool Connection::allocate(HighQueue::Message & message)
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
