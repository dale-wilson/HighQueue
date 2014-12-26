/// @file IvConnection.cpp
#include <Common/MPassPch.h>
#include "IvConnection.h"
#include <InfiniteVector/IvAllocator.h>
#include <InfiniteVector/IvEntry.h>

using namespace MPass;
using namespace InfiniteVector;

IvConnection::IvConnection()
{
}

void IvConnection::CreateLocal(const std::string & name, const IvCreationParameters & parameters)
{
    const size_t allocatedSize = spaceNeeded(parameters) + CacheLineSize;
    localMemory_.reset(new byte_t[allocatedSize]);
    byte_t * block = localMemory_.get();
    byte_t * alignedBlock = IvAllocator::align(block, CacheLineSize);
    Offset headerOffset = alignedBlock - block;
    size_t availableSize = allocatedSize - headerOffset;

    IvAllocator allocator(allocatedSize, sizeof(IvHeader));

    auto header = block + headerOffset;
    header_ = new (header) IvHeader(name, allocator, parameters);
    bemoryBlockAllocator_.reset(new Buffers::MemoryBlockAllocator(header, header_->blockInfo_));
}

IvHeader * IvConnection::getHeader() const
{
    return header_;
}
            
void IvConnection::OpenOrCreateShared(const std::string & name, const IvCreationParameters & parameters)
{
}

void IvConnection::OpenExistingShared(const std::string & name)
{
}

size_t IvConnection::spaceNeeded(const IvCreationParameters & parameters)
{
    size_t headerSize = IvAllocator::align(sizeof(IvHeader), CacheLineSize);
    size_t entriesSize = IvEntry::alignedSize() * parameters.entryCount_;
    size_t positionsSize = CacheLineSize; // note the assumption that positions fit in a single cache line
    size_t bufferPoolSize = parameters.bufferCount_ * IvAllocator::align(parameters.bufferSize_, CacheLineSize);
    // producers?
    // Consumer wait strategy?
    return headerSize + entriesSize + positionsSize + bufferPoolSize;
}

bool IvConnection::allocate(Buffers::Buffer & buffer)
{
    return bemoryBlockAllocator_->allocate(buffer);
}

void IvConnection::free(Buffers::Buffer & buffer)
{
    bemoryBlockAllocator_->free(buffer);
}
size_t IvConnection::getBufferSize()const
{
    return bemoryBlockAllocator_->getBufferSize();
}
size_t IvConnection::getBufferCount()const
{
    return bemoryBlockAllocator_->getBufferCount();
}
bool IvConnection::hasBuffers() const
{
    return bemoryBlockAllocator_->hasBuffers();
}
