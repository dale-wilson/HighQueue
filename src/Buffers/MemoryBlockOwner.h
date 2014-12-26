#pragma once

#include <Buffers/MemoryBlockAllocator.h>

namespace MPass
{
	namespace Buffers
	{
        class MemoryBlockOwner: public Buffer::MemoryOwner, public std::enable_shared_from_this<MemoryBlockOwner>
        {
        public:
            /// @brief Construct -- allocating internal memory
            MemoryBlockOwner(size_t bufferSize,size_t bufferCount);

            bool allocate(Buffer & buffer);
            void release(Buffer & buffer);
            size_t getBufferSize()const;
            size_t getStorageSize()const;
            byte_t const * getStorageAddress()const;
            size_t getBufferCount()const;
            bool hasBuffers() const;

        private:
            size_t blockSize_;
            std::unique_ptr<byte_t> block_;
            MemoryBlockAllocator allocator_;
        };
        typedef std::shared_ptr<MemoryBlockOwner> MemoryBlockOwnerPtr;
	}
}
