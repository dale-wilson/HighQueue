// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Buffers/MemoryBlockPool.h>
#include <Buffers/Buffer.h>

namespace MPass
{
	namespace Buffers
	{
        class MemoryBlockAllocator
		{
        public:

        public:
            // /// @brief Construct -- using external memory
            // MemoryBlockAllocator(byte_t * baseAddress, size_t blockSize, size_t bufferSize);
            
            /// @brief Construct -- using an existing MemoryBlockPool.
            MemoryBlockAllocator(byte_t * baseAddress, MemoryBlockPool & pool);

            ///// @brief Construct -- initializing a preallocated, but uninitialized MemoryBlockPool
            //MemoryBlockAllocator(byte_t * baseAddress, size_t blockSize, size_t offsetWithinBlock, MemoryBlockPool & container, size_t bufferSize);

            /// @brief Allocate a block of memory into a buffer.  
            /// @param buffer is the Buffer to receive the block of memory.
            /// @param owner will be notified when the buffer is released.
            bool allocate(Buffer & buffer, const Buffer::MemoryOwnerPtr & owner = Buffer::MemoryOwnerPtr());

            void release(Buffer & buffer);

            size_t getBufferCapacity()const;
            size_t getStorageSize()const;
            byte_t const * getStorageAddress()const;
            MemoryBlockPool & getContainer();
            const MemoryBlockPool & getContainer() const;
            size_t getBufferCount()const;
            bool hasMemoryAvailable() const;

        private:
            byte_t * baseAddress_;
            MemoryBlockPool & memoryPool_;
		};
	}
}
