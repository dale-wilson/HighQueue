/// @file IvConsumer.h
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
        /// @brief Support for consuming messages from an InfiniteVector
        /// In addition to having the IvConnection which is used to construct
        /// this object, you will need a Buffers::Buffer which has been initialized
        /// by calling the IvConnection::allocate() method.
		class IvConsumer
		{
		public:
            /// @brief Construct and attach to a connection
            /// @param connection provides access to the InfiniteVector
			IvConsumer(IvConnection & connection);

            /// @brief Get the next buffer-full of data if it is available
            ///
            /// You must call one of the Buffer::get() methods after a successful return
            /// from this call in order to access the data.  Do NOT save the result
            /// from a previous Buffer::get() call.  It will be invalidated by this call.
            ///
            /// @param buffer The buffer will be populated from the InfiniteVector entry.
            /// @returns immediately.  true if the buffer now contains data; false if no data is available.
            bool tryGetNext(Buffers::Buffer & buffer);

            /// @brief Get the next buffer-full of data.  Wait if none is available
            ///
            /// You must call one of the Buffer::get() methods after this call returns
            /// in order to access the data.  Do NOT save the result
            /// from a previous Buffer::get() call.  It will be invalidated by this call.
            ///
            /// @param buffer The buffer will be populated from the InfiniteVector entry.
            /// Note: uses the IvConsumerWaitStrategy to wait.
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