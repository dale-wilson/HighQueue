// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Buffers/Buffer.h>
#include <Common/Spinlock.h>

namespace MPass
{
	namespace Buffers
	{
        /// @brief A pool of memory blocks of the same size. 
        ///
        /// The MemoryBlockPool stores the parameters need to manage a pool of memory blocks.
        /// The pool is pointed to by a base address, but that address is not stored in the 
        /// MemoryBlockPool, nor is it stored in the memory managed by the MemoryBlockPool.
        /// Instead offsets within the block are used to identify memory blocks.
        /// This allows a pool to reside in shared memory which might be mapped
        /// to different addresses in different processes.
        struct MemoryBlockPool
        {
            /// @brief The size of the entire block of memory from which the memory is allocated.
            size_t blockSize_;

            /// @brief The size of each chuck of memory in the block. 
            size_t bufferSize_;
            
            /// @brief The total number of chunks in the block (constant -- does not change as memory is allocated or freed)
            size_t bufferCount_;
            
            /// @brief The root of a linked list.  This is an offset to the block origin. 
            size_t rootOffset_;

            /// @brief Synchronize access to rootOffset_
            Spinlock lock_;

            /// @brief Construct an empty info.
            /// Used in a placement new to interpret a location in memory as a MemoryBlockPool.
            /// This is particularly useful when the MemoryBlockPool is in shared memory that was initialized by another process.
            MemoryBlockPool();

            /// @brief Construct a MemoryBlockPool and initialize a block
            MemoryBlockPool(byte_t * baseAddress, size_t blockSize, size_t bufferSize, size_t initialOffset = 0);

            /// @brief Do not allow copies
            MemoryBlockPool(const MemoryBlockPool &) = delete;
            /// @brief Do not allow assignment
            MemoryBlockPool & operator=(const MemoryBlockPool &) = delete;

            /// @brief Initialize a block.
            size_t preAllocate(byte_t * baseAddress, size_t initialOffset/* = 0u*/);
            
            /// @brief Allocate a block of memory into a buffer.  
            /// @param baseAddress is the address used to resolve the offsets into actual addresses.
            /// @param buffer is the Buffer to receive the block of memory.
            bool allocate(byte_t * baseAddress, Buffer & buffer, const Buffer::MemoryOwnerPtr & owner = Buffer::MemoryOwnerPtr());

            /// @brief Free the block of memory from a buffer.
            /// @param baseAddress is the address used to resolve the offsets into actual addresses.
            /// @param buffer is the Buffer from which the memory will be returned.
            /// @throws runtime_error if the memory did not come from this block.
            void release(byte_t * baseAddress, Buffer & buffer);

            /// @brief Helper function to round a buffer size up to the next cache-line boundary.
            static size_t cacheAlignedBufferSize(size_t bufferSize);

            /// @brief Helper function to calculates how much space is needed to allocate 'bufferCount' cache aligned buffers 
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
        };
	}
}
