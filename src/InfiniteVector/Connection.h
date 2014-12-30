/// @file Connection.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <InfiniteVector/details/IvHeader.h>
#include <InfiniteVector/CreationParameters.h>
#include <InfiniteVector/details/MemoryBlockPool.h>

namespace MPass
{
	namespace InfiniteVector
	{
        /// @brief The application's connection to an InfiniteVector.
        /// The Connection is at the core of the InfiniteVector API.
        /// The first step in using an InfiniteVector is to construct
        /// an Connection, then use it to create or find the
        /// actual InfiniteVector.
        /// Once the connection is established and the clients (Producer
        /// and/or Consumer) are created, the allocate method provides
        /// access to the memory pool to be used by the messages.
        /// @see Producer
        /// @see Consumer
		class Connection
		{
		public:
            /// @brief Construct.
			Connection();
            
            ~Connection();

            /// @brief Allocate a block of local memory and create an InfiniteVector in that block. 
            void createLocal(
                const std::string & name, 
                const CreationParameters & parameters);
            
            /// @brief Attempt to attach to an existing InfiniteVector in shared memory.  If none
            /// is found, then create a new one.
            void openOrCreateShared(
                const std::string & name, 
                const CreationParameters & parameters); 

            /// @brief Connect to an existing InfiniteVector in shared memory.
            void openExistingShared(const std::string & name);

            /// @brief Populate a message with data from the InfiniteVector's memory pool
            /// @param the message to be populated.
            /// @returns true if there was memory available.
            bool allocate(InfiniteVector::Message & message);

            /// @brief Get the capacity of all messages used with this InfiniteVector
            size_t getMessageCapacity()const;
            
            /// @brief How many messages can this InfiniteVector support?
            /// Note this is a constant.  It does not change as memory is allocated or freed.
            size_t getMessageCount()const;
            
            /// @brief Is there enough memory available to populate a message?
            bool hasMemoryAvailable() const;

            /// @brief Provide direct access to internal implementation details.
            IvHeader * getHeader() const;
            
            /// @brief A helper function to determine how much space is needed in an block of memory
            /// large enough to hold an InfiniteVector
            static size_t spaceNeeded(const CreationParameters & parameters);

        private:
            boost::shared_array<byte_t> localMemory_;
            IvHeader * header_;
            InfiniteVector::MemoryBlockPool * memoryPool_;
		};
	}
}