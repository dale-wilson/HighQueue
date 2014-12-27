// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Buffers/Buffer.h>

namespace MPass
{
	namespace Buffers
	{
        /// @brief Information about a block from which memory can be allocated in fixed-size chunks.
        ///
        /// The MemoryBlockInfo stores the parameters need to manage a block of memory.  The block is pointed
        /// to by a base address, but that address is not stored in the MemoryBlockInfo, nor
        /// is it stored in the block of memory managed by the MemoryBlockInfo.
        /// Instead offsets within the block are used to identify buffers.
        /// This allows a meory block to reside in shared memory which might be mapped
        /// to different addresses in different processes.
        struct MemoryBlockInfo
        {
            /// @brief The size of the entire block of memory from which the memory is allocated.
            size_t blockSize_;

            /// @brief The size of each chuck of memory in the block. 
            size_t bufferSize_;
            
            /// @brief The total number of chunks in the block (constant -- does not change as memory is allocated or freed)
            size_t bufferCount_;
            
            /// @brief The root of a linked list.  This is an offset to the block origin. 
            size_t rootOffset_;

            /// @brief Construct an empty info.
            /// Used in a placement new to interpret a location in memory as a MemoryBlockInfo.
            /// This is particularly useful when the MemoryBlockInfo is in shared memory that was initialized by another process.
            MemoryBlockInfo();

            /// @brief Construct a MemoryBlockInfo and initialize a block
            MemoryBlockInfo(size_t blockSize, size_t bufferSize);

            /// @brief Internal method used to initialize a block.
            size_t preAllocate(byte_t * baseAddress, size_t initialOffset/* = 0u*/);
            
            /// @brief Allocate a block of memory into a buffer.  
            /// @param baseAddress is the address used to resolve the offsets into actual addresses.
            /// @param buffer is the Buffer to receive the block of memory.
            bool allocate(byte_t * baseAddress, Buffer & buffer, const Buffer::MemoryOwnerPtr & owner = Buffer::MemoryOwnerPtr());

            /// @brief Free the block of memory from a buffer.
            /// @param baseAddress is the address used to resolve the offsets into actual addresses.
            /// @param buffer is the Buffer from which the memory will be returned.
            /// @throws runtime_error if the memory did not come from this block.
            void free(byte_t * baseAddress, Buffer & buffer);
        };
	}
}
