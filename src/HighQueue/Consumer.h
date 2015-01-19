/// @file Consumer.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.h>
#include <HighQueue/Connection.h>
#include <HighQueue/details/HQResolver.h>
#include <HighQueue/details/HQReservePosition.h>
#include <HighQueue/details/HQEntryAccessor.h>

namespace HighQueue
{
    /// @brief Support for consuming messages from an HighQueue
    /// In addition to having the Connection which is used to construct
    /// this object, you will need a Message which has been initialized
    /// by calling the Connection::allocate() method.
    class HighQueue_Export Consumer
	{
	public:
        /// @brief Construct and attach to a connection
        /// @param connection provides access to the HighQueue
		Consumer(ConnectionPtr & connection);

        ~Consumer();

        /// @brief Get the next message-full of data if it is available
        ///
        /// You must call one of the Message::get() methods after a successful return
        /// from this call in order to access the data.  Do NOT save the result
        /// from a previous Message::get() call.  It will be invalidated by this call.
        ///
        /// @param message The message will be populated from the HighQueue entry.
        /// @returns immediately.  true if the message now contains data; false if no data is available.
        bool tryGetNext(Message & message);

        /// @brief Get the next message-full of data.  Wait if none is available
        ///
        /// You must call one of the Message::get() methods after this call returns
        /// in order to access the data.  Do NOT save the result
        /// from a previous Message::get() call.  It will be invalidated by this call.
        ///
        /// @param message The message will be populated from the HighQueue entry.
        /// @returns true unless shutting down.
        /// Note: uses the WaitStrategy to wait.
        bool getNext(Message & message);

        /// @brief for diagnosing and performance measurements, dump statistics
        std::ostream & writeStats(std::ostream & out)const;

        /// @brief stop receiving messages.
        void stop();

    private:
        void incrementReadPosition();
    private:
        ConnectionPtr connection_;
        HQHeader * header_;
        bool producerUsesMutex_;
        HighQResolver resolver_;
        HighQEntryAccessor entryAccessor_;
        volatile Position & readPosition_;
        volatile Position & publishPosition_;
        Position cachedPublishPosition_;
        const WaitStrategy & waitStrategy_;
        size_t spins_;
        size_t yields_;
        size_t sleeps_;

        bool stopping_;
        uint64_t statGets_;
        uint64_t statTrys_;
        uint64_t statSpins_;
        uint64_t statYields_;
        uint64_t statSleeps_;
        uint64_t statWaits_;
	};
}
