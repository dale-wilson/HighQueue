/// @file Consumer.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HSQueue_Export.h>
#include <HSQueue/Connection.h>
#include <HSQueue/details/HSQResolver.h>
#include <HSQueue/details/HSQReservePosition.h>
#include <HSQueue/details/HSQEntryAccessor.h>

namespace HSQueue
{
    /// @brief Support for consuming messages from an HSQueue
    /// In addition to having the Connection which is used to construct
    /// this object, you will need a HSQueue::Message which has been initialized
    /// by calling the Connection::allocate() method.
    class HSQueue_Export Consumer
	{
	public:
        /// @brief Construct and attach to a connection
        /// @param connection provides access to the HSQueue
		Consumer(Connection & connection);

        /// @brief Get the next message-full of data if it is available
        ///
        /// You must call one of the Message::get() methods after a successful return
        /// from this call in order to access the data.  Do NOT save the result
        /// from a previous Message::get() call.  It will be invalidated by this call.
        ///
        /// @param message The message will be populated from the HSQueue entry.
        /// @returns immediately.  true if the message now contains data; false if no data is available.
        bool tryGetNext(HSQueue::Message & message);

        /// @brief Get the next message-full of data.  Wait if none is available
        ///
        /// You must call one of the Message::get() methods after this call returns
        /// in order to access the data.  Do NOT save the result
        /// from a previous Message::get() call.  It will be invalidated by this call.
        ///
        /// @param message The message will be populated from the HSQueue entry.
        /// Note: uses the ConsumerWaitStrategy to wait.
        void getNext(HSQueue::Message & message);
    private:
        Connection & connection_;
        HSQHeader * header_;
        HSQResolver resolver_;
        HSQEntryAccessor entryAccessor_;
        volatile Position & readPosition_;
        volatile Position & publishPosition_;
        Position cachedPublishPosition_;
        const ConsumerWaitStrategy & waitStrategy_;
        size_t spins_;
        size_t yields_;
        size_t sleeps_;
	};
}
