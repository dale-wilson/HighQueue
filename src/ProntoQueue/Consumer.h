/// @file Consumer.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/Connection.h>
#include <ProntoQueue/details/PQResolver.h>
#include <ProntoQueue/details/PQReservePosition.h>
#include <ProntoQueue/details/IvEntryAccessor.h>

namespace MPass
{
	namespace ProntoQueue
	{
        /// @brief Support for consuming messages from an ProntoQueue
        /// In addition to having the Connection which is used to construct
        /// this object, you will need a ProntoQueue::Message which has been initialized
        /// by calling the Connection::allocate() method.
		class Consumer
		{
		public:
            /// @brief Construct and attach to a connection
            /// @param connection provides access to the ProntoQueue
			Consumer(Connection & connection);

            /// @brief Get the next message-full of data if it is available
            ///
            /// You must call one of the Message::get() methods after a successful return
            /// from this call in order to access the data.  Do NOT save the result
            /// from a previous Message::get() call.  It will be invalidated by this call.
            ///
            /// @param message The message will be populated from the ProntoQueue entry.
            /// @returns immediately.  true if the message now contains data; false if no data is available.
            bool tryGetNext(ProntoQueue::Message & message);

            /// @brief Get the next message-full of data.  Wait if none is available
            ///
            /// You must call one of the Message::get() methods after this call returns
            /// in order to access the data.  Do NOT save the result
            /// from a previous Message::get() call.  It will be invalidated by this call.
            ///
            /// @param message The message will be populated from the ProntoQueue entry.
            /// Note: uses the ConsumerWaitStrategy to wait.
            void getNext(ProntoQueue::Message & message);
        private:
            Connection & connection_;
            PQHeader * header_;
            PQResolver resolver_;
            IvEntryAccessor entryAccessor_;
            volatile Position & readPosition_;
            volatile Position & publishPosition_;
            Position cachedPublishPosition_;
            const ConsumerWaitStrategy & waitStrategy_;
            size_t spins_;
            size_t yields_;
            size_t sleeps_;
		};
	}
}