#pragma once

#include <Buffers/Buffer.h>
namespace MPass
{
	namespace Buffers
	{
        /// @brief Information about a block from which memory can be allocated.
        ///
        /// The MemoryBlockInfo stores the parameters need to manage a block of memory.  The block is pointed
        /// to by a base address, but that address is not stored in the MemoryBlockInfo, nor
        /// is it stored in the block of memory managed by the MemoryBlockInfo.
        /// Instead offsets within the block are used to identify buffers.
        /// This allows a meory block to reside in shared memory which might be mapped
        /// to different addresses in different processes.
        struct MemoryBlockInfo
        {
            size_t blockSize_;
            size_t bufferSize_;
            size_t bufferCount_;
            size_t rootOffset_;

            MemoryBlockInfo();
            MemoryBlockInfo(size_t blockSize, size_t bufferSize);
            size_t preAllocate(byte_t * baseAddress, size_t initialOffset/* = 0u*/);
            bool allocate(byte_t * baseAddress, Buffer & buffer, const Buffer::MemoryOwnerPtr & owner = Buffer::MemoryOwnerPtr());
            void free(byte_t * baseAddress, Buffer & buffer);
        };

	}
}
