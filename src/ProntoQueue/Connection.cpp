/// @file Connection.cpp
#include <Common/ProntoQueuePch.h>
#include "Connection.h"
#include <ProntoQueue/details/PQAllocator.h>
#include <ProntoQueue/details/PQEntry.h>
#include <ProntoQueue/details/PQResolver.h>

using namespace ProntoQueue;

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

void Connection::createLocal(const std::string & name, const CreationParameters & parameters)
{
    const size_t allocatedSize = spaceNeeded(parameters);
    localMemory_.reset(new byte_t[allocatedSize]);
    try
    {
        byte_t * block = localMemory_.get();
        byte_t * alignedBlock = PQAllocator::align(block, CacheLineSize);
        size_t availableSize = allocatedSize - (alignedBlock - block);

        PQAllocator allocator(availableSize, sizeof(PQHeader));
        header_ = new (alignedBlock)PQHeader(name, allocator, parameters);
        PQResolver resolver(header_);
        memoryPool_ = resolver.resolve<ProntoQueue::MemoryBlockPool>(header_->memoryPool_);
    }
    catch(...)
    {
        localMemory_.reset();
        header_ = 0;
        memoryPool_ = 0;
        throw;
    }
}

PQHeader * Connection::getHeader() const
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
    size_t headerSize = PQAllocator::align(sizeof(PQHeader), CacheLineSize);
    size_t entriesSize = PQEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that positions fit in a single cache line
    size_t messagePoolSize = ProntoQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    size_t cacheAlignmentSize = CacheLineSize;
    return headerSize + entriesSize + positionsSize + messagePoolSize + cacheAlignmentSize;
}

bool Connection::allocate(ProntoQueue::Message & message)
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
