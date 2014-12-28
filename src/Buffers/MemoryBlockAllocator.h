// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Buffers/MemoryBlockInfo.h>
#include <Buffers/Buffer.h>

namespace MPass
{
	namespace Buffers
	{
        class MemoryBlockAllocator
		{
        public:

        public:
            /// @brief Construct -- using external memory
            MemoryBlockAllocator(byte_t * baseAddress, size_t blockSize, size_t bufferSize);
            /// @brief Construct -- using an existing MemoryBlockInfo.
            MemoryBlockAllocator(byte_t * baseAddress, MemoryBlockInfo & container);
            /// @brief Construct -- initializing a preallocated, but uninitialized MemoryBlockInfo
            MemoryBlockAllocator(byte_t * block, size_t blockSize, size_t offsetWithinBlock, MemoryBlockInfo & container, size_t bufferSize);

            /// @brief Allocate a block of memory into a buffer.  
            /// @param buffer is the Buffer to receive the block of memory.
            /// @param owner will be notified when the buffer is released.
            bool allocate(Buffer & buffer, const Buffer::MemoryOwnerPtr & owner = Buffer::MemoryOwnerPtr());

            void release(Buffer & buffer);

            size_t getBufferCapacity()const;
            size_t getStorageSize()const;
            byte_t const * getStorageAddress()const;
            MemoryBlockInfo & getContainer();
            const MemoryBlockInfo & getContainer() const;
            size_t getBufferCount()const;
            bool hasMemoryAvailable() const;

            static size_t cacheAlignedBufferSize(size_t bufferSize);

            /// @brief Calculates how much space is needed to allocate 'bufferCount' cache aligned buffers 
            ///        of at least 'bufferSize' bytes each in an unaligned block of memory..
            ///
            /// Note:  Because this handles the worst-case: 
            ///        It is likely that the actual buffer sizes will exceed bufferSize, and
            ///        It is possible that the actual number of buffers will exceed bufferCount.
            ///
            /// @param bufferSize the minimum number of bytes per buffer.
            /// @param bufferCount the minimum number of buffers.
            /// @returns the number of bytes needed to insure that the size and count requrements can be met.
            static size_t spaceNeeded(size_t bufferSize, size_t bufferCount);

        private:
            byte_t * baseAddress_;
            MemoryBlockInfo internalContainer_;
            MemoryBlockInfo & container_;
		};
	}
}
