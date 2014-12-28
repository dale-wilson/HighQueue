// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Buffers/MemoryBlockAllocator.h>

namespace MPass
{
	namespace Buffers
	{
        /// @brief An object that manages an external memory block
        class MemoryBlockManager : public Buffer::MemoryOwner, public std::enable_shared_from_this<MemoryBlockManager>
        {
        public:
            /// @brief Construct -- using external memory
            MemoryBlockManager(byte_t * baseAddress, Buffers::MemoryBlockPool & blockInfo);

            /// @brief Populate a Buffer with memory from the block.
            bool allocate(Buffer & buffer);

            /// @brief Release the memory from a buffer back into the block
            /// @param buffer is the buffer from which memory will be released.
            /// @throws runtime_error if this memory did not come from this block.
            void release(Buffer & buffer);

            /// @brief Get the actual size of buffers allocated by this BlockOwner.
            /// @returns a value >= the bufferSize requested in the constructor.
            size_t getBufferCapacity()const;

            /// @brief Get the total number of buffers in the block 
            /// Note this is constant, it does not change as memory is allocated into or released from buffers.
            /// @returns a value >= bufferCount requested in the constructor.
            size_t getBufferCount()const;

            /// @brief Is enough space available for a successful call to allocate()?
            bool hasMemoryAvailable() const;

            /// @brief Get the total amount of storage allocated.
            /// This is not a very interesting number!
            /// @returns the amount of space allocated in order to satifisy the size and count contraints from the constructor.
            size_t getStorageSize()const;

            /// @brief Get the actual beginning of the block from which memory will be allocated.
            /// Not very interesting!
            /// @returns the origin of the memory block.
            byte_t const * getStorageAddress()const;

        private:
            MemoryBlockAllocator allocator_;
        };

        typedef std::shared_ptr<MemoryBlockManager> MemoryBlockManagerPtr;
	}
}
