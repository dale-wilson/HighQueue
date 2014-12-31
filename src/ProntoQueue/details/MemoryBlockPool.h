// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ProntoQueue/Message.h>
#include <Common/Spinlock.h>

namespace ProntoQueue
{
    /// @brief A pool of memory blocks of the same size. 
    ///
    /// The MemoryBlockPool stores the parameters need to manage a pool of memory blocks.
    /// The pool is pointed to by a base address, but that address is not stored in the 
    /// MemoryBlockPool, nor is it stored in the memory managed by the MemoryBlockPool.
    /// Instead offsets within the block are used to identify memory blocks.
    /// This allows a pool to reside in shared memory which might be mapped
    /// to different addresses in different processes.
    struct MemoryBlockPool
    {
        /// A flag to mark the end of the linked list of memory blocks.

        const size_t NULL_OFFSET = ~(size_t(0));
        /// @brief The size of the entire pool of memory
        size_t blockSize_;

        /// @brief The size of each block of memory in the pool. 
        size_t messageSize_;
            
        /// @brief The total number of block in the pool
        /// constant: does not change as memory is allocated or freed)
        size_t messageCount_;
            
        /// @brief The root of a linked list.  This is an offset to the pool base address
        size_t rootOffset_;

        /// @brief Synchronize access to rootOffset_
        Spinlock lock_;

        /// @brief Construct an empty pool.
        ///
        /// This is mostly useless, but there are occasions where it is needed to
        /// allocate a pool to be initialized "by hand" later.
        MemoryBlockPool();

        /// @brief Construct and initialize a MemoryBlockPool
        MemoryBlockPool(size_t blockSize, size_t messageSize);

        /// @brief Do not allow copies
        MemoryBlockPool(const MemoryBlockPool &) = delete;

        /// @brief Do not allow assignment
        MemoryBlockPool & operator=(const MemoryBlockPool &) = delete;

        /// @brief Allocate a block of memory into a message.  
        /// @param baseAddress is the address used to resolve the offsets into actual addresses.
        /// @param message is the Message to receive the block of memory.
        bool allocate(Message & message);

        /// @brief Free the block of memory from a message.
        /// @param baseAddress is the address used to resolve the offsets into actual addresses.
        /// @param message is the Message from which the memory will be returned.
        /// @throws runtime_error if the memory did not come from this block.
        void release(Message & message);

        /// @brief Are messages available?
        ///
        /// Warning.  This is not threadsafe.  If you really want to know, try to allocate.
        /// (this is here mostly for unit testing.)
        bool isEmpty() const;

        size_t getMessageCapacity()const
        {
            return messageSize_;
        }
        size_t getMessageCount()const
        {
            return messageCount_;
        }

        /// @brief Initialize a block.
        /// for internal use (and testing)
        size_t preAllocate(size_t messageSize, size_t blockSize);

        static MemoryBlockPool * makeNew(size_t messageSize, size_t messageCount);

        /// @brief Helper function to round a message size up to the next cache-line boundary.
        static size_t cacheAlignedMessageSize(size_t messageSize);

        /// @brief Helper function to calculates how much space is needed to allocate 'messageCount' cache aligned messages 
        ///        of at least 'messageSize' bytes each in an unaligned block of memory..
        ///
        /// Note:  Because this handles the worst-case: 
        ///        It is likely that the actual message sizes will exceed messageSize, and
        ///        It is possible that the actual number of messages will exceed messageCount.
        ///
        /// @param messageSize the minimum number of bytes per message.
        /// @param messageCount the minimum number of messages.
        /// @returns the number of bytes needed to insure that the size and count requrements can be met.
        static size_t spaceNeeded(size_t messageSize, size_t messageCount);
    };
}
