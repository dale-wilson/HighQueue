/// @file Connection.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Common/HighQueue_Export.h>
#include <HighQueue/details/HQHeader.h>
#include <HighQueue/CreationParameters.h>
#include <HighQueue/details/MemoryBlockPool.h>

namespace HighQueue
{
    /// @brief The application's connection to an HighQueue.
    /// The Connection is at the core of the HighQueue API.
    /// The first step in using an HighQueue is to construct
    /// an Connection, then use it to create or find the
    /// actual HighQueue.
    /// Once the connection is established and the clients (Producer
    /// and/or Consumer) are created, the allocate method provides
    /// access to the memory pool to be used by the messages.
    /// @see Producer
    /// @see Consumer
    class HighQueue_Export Connection
	{
	public:
        /// @brief Construct.
		Connection();
            
        ~Connection();

        /// @brief Allocate a block of local memory and create an HighQueue in that block. 
        void createLocal(
            const std::string & name, 
            const CreationParameters & parameters);
            
        /// @brief Attempt to attach to an existing HighQueue in shared memory.  If none
        /// is found, then create a new one.
        void openOrCreateShared(
            const std::string & name, 
            const CreationParameters & parameters); 

        void close();

        /// @brief Connect to an existing HighQueue in shared memory.
        void openExistingShared(const std::string & name);

        /// @brief Populate a message with data from the HighQueue's memory pool
        /// @param the message to be populated.
        /// @returns true if there was memory available.
        bool allocate(HighQueue::Message & message);

        /// @brief Get the capacity of all messages used with this HighQueue
        size_t getMessageCapacity()const;
            
        /// @brief How many messages can this HighQueue support?
        /// Note this is a constant.  It does not change as memory is allocated or freed.
        size_t getMessageCount()const;
            
        /// @brief Is there enough memory available to populate a message?
        bool hasMemoryAvailable() const;

        /// @brief Provide direct access to internal implementation details.
        HQHeader * getHeader() const;
            
        /// @brief A helper function to determine how much space is needed in an block of memory
        /// large enough to hold an HighQueue
        static size_t spaceNeeded(const CreationParameters & parameters);

    private:
        std::shared_ptr<byte_t> localMemory_;
        HQHeader * header_;
        HighQueue::MemoryBlockPool * memoryPool_;
	};
}
