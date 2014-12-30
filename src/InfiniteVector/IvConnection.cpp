/// @file IvConnection.cpp
#include <Common/MPassPch.h>
#include "IvConnection.h"
#include <InfiniteVector/IvAllocator.h>
#include <InfiniteVector/IvEntry.h>
#include <InfiniteVector/IvResolver.h>

using namespace MPass;
using namespace InfiniteVector;

IvConnection::IvConnection()
{
}

IvConnection::~IvConnection()
{
    if(localMemory_ && header_)
    {
        header_->releaseInternalMessages();
    }
}

void IvConnection::createLocal(const std::string & name, const IvCreationParameters & parameters)
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

IvHeader * IvConnection::getHeader() const
{
    return header_;
}
            
void IvConnection::openOrCreateShared(const std::string & name, const IvCreationParameters & parameters)
{
}

void IvConnection::openExistingShared(const std::string & name)
{
}

size_t IvConnection::spaceNeeded(const IvCreationParameters & parameters)
{
    size_t headerSize = IvAllocator::align(sizeof(IvHeader), CacheLineSize);
    size_t entriesSize = IvEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize * 3; // note the assumption that positions fit in a single cache line
    size_t messagePoolSize = InfiniteVector::MemoryBlockPool::spaceNeeded(parameters.messageSize_, parameters.messageCount_);
    return headerSize + entriesSize + positionsSize + messagePoolSize;
}

bool IvConnection::allocate(InfiniteVector::Message & message)
{
    return memoryPool_->allocate(message);
}

size_t IvConnection::getMessageCapacity()const
{
    return memoryPool_->getMessageCapacity();
}
size_t IvConnection::getMessageCount()const
{
    return memoryPool_->getMessageCount();
}
bool IvConnection::hasMemoryAvailable() const
{
    return !memoryPool_->isEmpty();
}
