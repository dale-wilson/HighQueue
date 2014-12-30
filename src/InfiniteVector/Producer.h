/// @file Producer.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <InfiniteVector/Connection.h>
#include <InfiniteVector/details/IvResolver.h>
#include <InfiniteVector/details/IvReservePosition.h>
#include <InfiniteVector/details/IvEntryAccessor.h>

namespace MPass
{
	namespace InfiniteVector
	{
        /// @brief Support for publishing messages to an InfiniteVector
        /// In addition to having the Connection which is used to construct
        /// this object, you will need a InfiniteVector::Message which has been initialized
        /// by calling the Connection::allocate() method.
		class Producer
		{
        public:
            /// @brief Construct and attach to a connection
            /// @param connection provides access to the InfiniteVector
            /// @param solo indicates that this is the only producer.
            ///        solo producers run faster using techniques that would be 
            ///        unsafe with multiple producers.
			explicit Producer(Connection & connection, bool solo = false);  

            /// @brief Destructor
            ~Producer();
            
            /// @brief Publish the data contained in a message.
            ///
            /// If the visible window of the InfiniteVector is full this call
            /// waits forever.  (todo: fix this!)
            ///
            /// After this call the message will point to a different (unused)
            /// area in memory.  You must call Message::get() to find this memory
            /// area.  Do not save the result of a previous Message::get() call,
            /// this call invalidates previous get() results.
            ///
            /// @param message contains the data to be published.         
            void publish(InfiniteVector::Message & message);

        private:
            uint64_t reserve();
        private:
            bool solo_;
            Connection & connection_;
            IvHeader * header_;
            IvResolver resolver_;
            volatile Position & readPosition_;
            volatile Position & publishPosition_;
            volatile AtomicPosition & reservePosition_;
            volatile Position & reserveSoloPosition_;
            Spinlock & reserveSpinLock_;
            IvEntryAccessor entryAccessor_;
		};
	}
}