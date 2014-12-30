/// @file IvProducer.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <InfiniteVector/IvConnection.h>
#include <InfiniteVector/IvResolver.h>
#include <InfiniteVector/IvReservePosition.h>
#include <InfiniteVector/IvEntryAccessor.h>

namespace MPass
{
	namespace InfiniteVector
	{
        /// @brief Support for publishing messages to an InfiniteVector
        /// In addition to having the IvConnection which is used to construct
        /// this object, you will need a InfiniteVector::Buffer which has been initialized
        /// by calling the IvConnection::allocate() method.
		class IvProducer
		{
        public:
            /// @brief Construct and attach to a connection
            /// @param connection provides access to the InfiniteVector
			IvProducer(IvConnection & connection);  
            
            /// @brief Publish the data contained in a buffer.
            ///
            /// If the visible window of the InfiniteVector is full this call
            /// waits forever.  (todo: fix this!)
            ///
            /// After this call the buffer will point to a different (unused)
            /// area in memory.  You must call Buffer::get() to find this memory
            /// area.  Do not save the result of a previous Buffer::get() call,
            /// this call invalidates previous get() results.
            ///
            /// @param buffer contains the data to be published.         
            void publish(InfiniteVector::Buffer & buffer);
        private:
            IvConnection & connection_;
            IvHeader * header_;
            IvResolver resolver_;
            volatile Position & readPosition_;
            volatile Position & publishPosition_;
            volatile AtomicPosition & reservePosition_;
            IvEntryAccessor entryAccessor_;
		};
	}
}