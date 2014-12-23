/// @file IvConsumer.h
#pragma once
#include <InfiniteVector/IvConnection.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvReservePosition.h>
#include <InfiniteVector/IvEntryAccessor.h>
namespace MPass
{
	namespace InfiniteVector
	{
		class IvConsumer
		{
		public:
			IvConsumer(IvConnection & connection);

            bool tryGetNext(Buffers::Buffer & buffer);
            void getNext(Buffers::Buffer & buffer);
        private:
            IvConnection & connection_;
            IvHeader * header_;
            IvResolver resolver_;
            volatile Position & readPosition_;
            volatile Position & publishPosition_;
            IvEntryAccessor entryAccessor_;

            Position publishedPosition_;

		};
	}
}