/// @file MemoryPool.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "MemoryPoolFwd.h"
#include <HighQueue/details/HQMemoryBLockPool.h>
#include <HighQueue/MessageFwd.h>

namespace HighQueue
{
    class HighQueue_Export MemoryPool
    {
    public:
        /// @brief Construct.
        MemoryPool(size_t blockSize, size_t count);

        ~MemoryPool();

        /// @brief Populate a message with a block from the HighQueue's memory pool
        /// @param the message to be populated.
        /// @returns true if there was memory available.
        bool tryAllocate(Message & message);

        /// @brief Populate a message with a block from the HighQueue's memory pool
        /// @param the message to be populated.
        /// @throws runtime_error if no memory was available
        void allocate(Message & message);

        /// @brief Get the capacity of each block in this pool
        size_t getBlockCapacity()const;

        HQMemoryBLockPool & getPool()
        {
            return pool_;
        }

    private:
        size_t allocatedSize_;
        std::shared_ptr<byte_t> memory_;
        HQMemoryBLockPool & pool_;


        };
}
