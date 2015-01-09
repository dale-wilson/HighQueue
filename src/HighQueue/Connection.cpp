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
    if(memoryPool_ && header_)
    {
        header_->releaseInternalMessages();
    }
}

void Connection::close()
{
    if(memoryPool_ && header_)
    {
        header_->releaseInternalMessages();
    }
}

void Connection::createLocal(
      const std::string & name, 
      const CreationParameters & parameters, 
      const MemoryPoolPtr & pool)
{
    memoryPool_ = pool;
    if(!memoryPool_)
    {
        memoryPool_.reset(new MemoryPool(parameters.messageSize_, parameters.messageCount_));
    }

    const size_t allocatedSize = spaceNeededForHeader(parameters);
    queueMemory_.reset(new byte_t[allocatedSize]);
    try
    {
        byte_t * block = queueMemory_.get();
        byte_t * alignedBlock = HQAllocator::align(block, CacheLineSize);
        size_t availableSize = allocatedSize - (alignedBlock - block);

        HQAllocator allocator(availableSize, sizeof(HQHeader));
        header_ = new (alignedBlock)HQHeader(name, allocator, parameters, &memoryPool_->getPool());
    }
    catch(...)
    {
        memoryPool_.reset();
        queueMemory_.reset();
        header_ = 0;
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

size_t Connection::spaceNeededForHeader(const CreationParameters & parameters)
{
    size_t headerSize = HQAllocator::align(sizeof(HQHeader), CacheLineSize);
    size_t entriesSize = HighQEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that each position fitx in a single cache line
    size_t cacheAlignmentSize = CacheLineSize;
    return headerSize + entriesSize + positionsSize + CacheLineSize;
}

void Connection::allocate(Message & message)
{
    memoryPool_->allocate(message);
}

bool Connection::tryAllocate(Message & message)
{
    return memoryPool_->tryAllocate(message);
}

size_t Connection::getMessageCapacity()const
{
    return memoryPool_->getBlockCapacity();
}
