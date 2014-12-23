#pragma once

#include <Buffers/Buffer.h>
namespace MPass
{
	namespace Buffers
	{
		class BufferAllocator
		{
        public:
            /// @brief A container for buffers.
            ///
            /// The BufferContainer manages a memory block full of buffers.  The block is pointed
            /// to by a base address, but that address is not stored in the BufferContainer, nor
            /// is it stored in the block of memory managed by the BufferContainer.
            /// Instead offsets within the block are used to identify buffers.
            /// This allows a BufferContainer to reside in shared memory which might be mapped
            /// to different addresses in different processes.
            struct BufferContainer
            {            
                size_t blockSize_;
                size_t bufferSize_;
                size_t bufferCount_;
                size_t rootOffset_;

                BufferContainer();
                BufferContainer(size_t blockSize, size_t bufferSize);
                size_t preAllocate(byte_t * baseAddress, size_t initialOffset/* = 0u*/);
                bool allocate(byte_t * baseAddress, Buffer & buffer);
                void free(byte_t * baseAddress, Buffer & buffer);
            };

        public:
            /// @brief Construct -- allocating internal memory
			BufferAllocator(size_t bufferSize, size_t bufferCount);
            /// @brief Construct -- using external memory
            BufferAllocator(size_t bufferSize, byte_t * block, size_t blockSize);
            /// @brief Construct -- using a shared BufferContainer.
            BufferAllocator(byte_t * baseAddress, BufferContainer & container);
            /// @brief Construct -- initializing a shared BufferContainer
            BufferAllocator(byte_t * block, size_t blockSize, size_t offsetWithinBlock, BufferContainer & container, size_t bufferSize);

			bool allocate(Buffer & buffer);
			void free(Buffer & buffer);

            size_t getBufferSize()const
            {
                return container_.bufferSize_;
            }

            size_t getStorageSize()const
            {
                return container_.blockSize_;
            }

            byte_t const * getStorageAddress()const
            {
                return block_.get();
            }

            size_t getBufferCount()const
            {
                return container_.bufferCount_;
            }

            bool hasBuffers() const
            {
                return container_.rootOffset_ + container_.bufferSize_ <= container_.blockSize_;
            }

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
            std::unique_ptr<byte_t> block_;
            byte_t * baseAddress_;
            BufferContainer internalContainer_;
            BufferContainer & container_;
		};
	}
}
