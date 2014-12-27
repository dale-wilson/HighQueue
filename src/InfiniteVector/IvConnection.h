/// @file IvConnection.h
#pragma once
#include <InfiniteVector/IvHeader.h>
#include <InfiniteVector/IvCreationParameters.h>
#include <Buffers/MemoryBlockAllocator.h>

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

            IvHeader * getHeader() const;
            
        private:
            boost::shared_array<byte_t> localMemory_;
            IvHeader * header_;
            std::shared_ptr<Buffers::MemoryBlockAllocator> bemoryBlockAllocator_;
		};
	}
}