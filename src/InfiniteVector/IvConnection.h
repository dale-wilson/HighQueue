/// @file IvConnection.h
#pragma once
#include <InfiniteVector/IvHeader.h>
#include <InfiniteVector/IvCreationParameters.h>
#include <Buffers/BufferAllocator.h>

namespace MPass
{
	namespace InfiniteVector
	{
		class IvConnection
		{
		public:
			IvConnection();
            void CreateLocal(
                const std::string & name, 
                const IvCreationParameters & parameters);
            void OpenOrCreateShared(
                const std::string & name, 
                const IvCreationParameters & parameters); 
            void OpenExistingShared(const std::string & name);

            size_t spaceNeeded(const IvCreationParameters & parameters);

            bool allocate(Buffers::Buffer & buffer);
			void free(Buffers::Buffer & buffer);
            size_t getBufferSize()const;
            size_t getBufferCount()const;
            bool hasBuffers() const;

            const IvHeader & getHeader() const
            {
                return *header_;
            }
            
        private:
            std::unique_ptr<byte_t[]> localMemory_;
            IvHeader * header_;
            std::unique_ptr<Buffers::BufferAllocator> bufferAllocator_;
		};
	}
}