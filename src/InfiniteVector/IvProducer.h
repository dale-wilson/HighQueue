/// @file IvProducer.h
#pragma once
#include <InfiniteVector/IvConnection.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvReservePosition.h>
#include <InfiniteVector/IvEntryAccessor.h>

namespace MPass
{
	namespace InfiniteVector
	{
		class IvProducer
		{
        public:
			IvProducer(IvConnection & connection);           
            void publish(Buffers::Buffer & buffer);
        private:
            IvConnection & connection_;
            IvHeader * header_;
            IvResolver resolver_;
            volatile Position * readPosition_;
            volatile Position * publishPosition_;
            volatile IvReservePosition * reservePosition_;
            IvEntryAccessor entryAccessor_;
		};
	}
}