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
        class MemoryBlockAllocator: public Buffer::MemoryOwner, public std::enable_shared_from_this<MemoryBlockAllocator>
		{
        public:

        public:
            /// @brief Construct -- using an existing MemoryBlockPool.
            MemoryBlockAllocator(byte_t * baseAddress, MemoryBlockPool & pool);

            /// @brief Allocate a block of memory into a buffer.  
            /// @param buffer is the Buffer to receive the block of memory.
            /// This will act as the buffer's owner
            bool allocate(Buffer & buffer);

            /// @brief Allocate a block of memory into a buffer.  
            /// @param buffer is the Buffer to receive the block of memory.
            /// @param owner will be notified when the buffer is released.
            bool allocate(Buffer & buffer, Buffer::MemoryOwnerPtr & owner);

            /// @brief Implement MemoryOwner method
            virtual void release(Buffer & buffer);

            size_t getBufferCapacity()const;
            size_t getStorageSize()const;
            byte_t const * getStorageAddress()const;
            MemoryBlockPool & getContainer();
            const MemoryBlockPool & getContainer() const;
            size_t getBufferCount()const;
            /// @brief Are we out of buffer space?
            bool isEmpty() const;

        private:
            byte_t * baseAddress_;
            MemoryBlockPool & memoryPool_;
		};
        typedef std::shared_ptr<MemoryBlockAllocator> MemoryBlockAllocatorPtr;
	}
}
