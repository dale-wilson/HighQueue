// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Buffers/MemoryBlockAllocator.h>

namespace MPass
{
	namespace Buffers
	{
        /// @brief An object that owns a memory block.  Note the MemoryBlockAllocator actually manages the memory block.
        class MemoryBlockHolder: public Buffer::MemoryOwner, public std::enable_shared_from_this<MemoryBlockHolder>
        {
        public:
            /// @brief Construct -- allocating internal memory and dividing it up into fixed size buffers.
            /// @param bufferSize is the minimum size of each buffer.  Rounded up to a cache-line size so actual buffers will usually be larger. 
            /// @param bufferCount is the minumum number of buffers needed.  Due to alignment issues there may be an extra buffer.
            MemoryBlockHolder(size_t bufferSize, size_t bufferCount);

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
            size_t blockSize_;
            std::unique_ptr<byte_t> block_;
            MemoryBlockAllocator allocator_;
        };
        typedef std::shared_ptr<MemoryBlockHolder> MemoryBlockHolderPtr;
	}
}
