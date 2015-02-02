// Copyright (c) 2014, 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "ConnectionFwd.h"
#include <Common/HighQueue_Export.h>
#include <HighQueue/details/HQHeader.h>
#include <HighQueue/CreationParameters.h>
#include <HighQueue/MemoryPool.h>

namespace HighQueue
{
    /// @brief The application's connection to an HighQueue.
    ///
    /// The Connection is the entry point the HighQueue API.
    /// The first step in using an HighQueue is to construct
    /// an Connection, then use it to create or find the
    /// actual HighQueue.
    /// Once the connection is established the ConnectionPtr
    /// can be passed as a construction argument to clients (Producerx
    /// and Consumer) and Messages.
    /// @see HighQueue::Producer
    /// @see HighQueue::Consumer
    /// @see HighQueue::Message
    class HighQueue_Export Connection
    {
    public:
        /// @brief Construct.
        Connection();
            
        ~Connection();

        /// @brief Allocate a block of local memory and create an HighQueue in that block. 
        /// @param name identifies the HighQueue.  For local queues this is for documentation only
        /// @param parameters configure the operation of the HighQueue
        /// @param pool is a shared memory pool from which Message buffers will be allocated.
        ///             if pool is null, a new pool will be constructed inside the HighQueue memory block.
        void createLocal(
            const std::string & name, 
            const CreationParameters & parameters,
            const MemoryPoolPtr & pool = MemoryPoolPtr());
            
        /// @brief Attempt to attach to an existing HighQueue in shared memory.  If none
        /// is found, then create a new one.
        /// @param name Identifies this shared memory block so other processes can find it.
        /// @param parameters configure the operation of the HighQueue
        void openOrCreateShared(
            const std::string & name, 
            const CreationParameters & parameters); 

        /// @brief CLose the connection to the HighQueue
        /// @deprecated  Turns out this is not really needed.  Just let the Connection go out-of-scope.
        void close();

        /// @brief Connect to an existing HighQueue in shared memory.
        /// @param name will be used to find the shared memory block.
        void openExistingShared(const std::string & name);

        /// @brief Populate a message with a block from the HighQueue's memory pool
        /// @param the message to be populated.
        /// @returns true if there was memory available.
        bool tryAllocate(Message & message);

        /// @brief Populate a message with a block from the HighQueue's memory pool
        /// @param the message to be populated.
        /// @throws runtime_error if no memory was available
        void allocate(Message & message);

        /// @brief Get the capacity of each message used with this HighQueue
        size_t getMessageCapacity()const;
            
        /// @brief Provide direct access to internal implementation details.
        HQHeader * getHeader() const;
            
        /// @brief A helper function to determine how much space is needed in an block of memory
        /// large enough to hold an HighQueue.  The value does NOT include the size of a memory pool
        /// if one is needed.
        /// @param parameters will be used to create the HighQueue
        /// @returns a byte count suitable for use in "new byte_t[count]" or even malloc.
        static size_t spaceNeededForHeader(const CreationParameters & parameters);

        void willProduce();
        bool canSolo()const;
    private:
        size_t expectedProducers_;
        MemoryPoolPtr memoryPool_;
        std::unique_ptr<byte_t[]> queueMemory_; 
        HQHeader * header_;
    };
}

