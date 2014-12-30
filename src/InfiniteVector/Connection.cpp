/// @file Connection.cpp
#include <Common/MPassPch.h>
#include "Connection.h"
#include <InfiniteVector/details/IvAllocator.h>
#include <InfiniteVector/details/IvEntry.h>
#include <InfiniteVector/details/IvResolver.h>

using namespace MPass;
using namespace InfiniteVector;

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
    byte_t * alignedBlock = IvAllocator::align(block, CacheLineSize);
    Offset headerOffset = Offset(alignedBlock - block);
    size_t availableSize = allocatedSize - headerOffset;

    IvAllocator allocator(allocatedSize, sizeof(IvHeader));

    auto header = block + headerOffset;
    header_ = new (header) IvHeader(name, allocator, parameters);
    IvResolver resolver(header_);
    memoryPool_ = resolver.resolve<InfiniteVector::MemoryBlockPool>(header_->memoryPool_);
}

IvHeader * Connection::getHeader() const
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
    size_t headerSize = IvAllocator::align(sizeof(IvHeader), CacheLineSize);
    size_t entriesSize = IvEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that positions fit in a single cache line
    size_t messagePoolSize = InfiniteVector::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    return headerSize + entriesSize + positionsSize + messagePoolSize;
}

bool Connection::allocate(InfiniteVector::Message & message)
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
