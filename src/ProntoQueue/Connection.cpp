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
    const size_t allocatedSize = spaceNeeded(parameters) + CacheLineSize;
    localMemory_.reset(new byte_t[allocatedSize]);
    byte_t * block = localMemory_.get();
    byte_t * alignedBlock = PQAllocator::align(block, CacheLineSize);
    Offset headerOffset = Offset(alignedBlock - block);
    size_t availableSize = allocatedSize - headerOffset;

    PQAllocator allocator(allocatedSize, sizeof(PQHeader));

    auto header = block + headerOffset;
    header_ = new (header) PQHeader(name, allocator, parameters);
    PQResolver resolver(header_);
    memoryPool_ = resolver.resolve<ProntoQueue::MemoryBlockPool>(header_->memoryPool_);
}

PQHeader * Connection::getHeader() const
{
    return header_;
}
            
void Connection::openOrCreateShared(const std::string & name, const CreationParameters & parameters)
{
}

void Connection::openExistingShared(const std::string & name)
{
}

size_t Connection::spaceNeeded(const CreationParameters & parameters)
{
    size_t headerSize = PQAllocator::align(sizeof(PQHeader), CacheLineSize);
    size_t entriesSize = PQEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that positions fit in a single cache line
    size_t messagePoolSize = ProntoQueue::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    return headerSize + entriesSize + positionsSize + messagePoolSize;
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
