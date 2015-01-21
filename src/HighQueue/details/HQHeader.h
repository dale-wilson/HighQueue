/// @file HQHeader.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.h>
#include <HighQueue/details/HQDefinitions.h>
#include <HighQueue/details/HQMemoryBlockPoolFwd.h>
#include <HighQueue/details/HQAllocator.h>
#include <HighQueue/CreationParameters.h>

namespace HighQueue
{
    /// @brief A structure to appear at the begining of a HighQueue
    /// 
    /// Addresses within the queue are stored as Offsets.  This allows the queue
    /// to function coreectly in shared memory even if it is mapped to different
    /// locations in different processes.
    ///
    struct HighQueue_Export HQHeader
    {
        const static Signature InitializingSignature = 0xFEEDBABE;
        const static Signature LiveSignature = 0xFEED600D;
        const static Signature DeadSignature = 0xFEEDD1ED;
        const static uint8_t Version = 0;

        /// @brief Verify that this is a HighQueue and that it is properly initialized
        Signature signature_;
        
        /// @brief Detect the version of this header to detect mismatches between programs sharing a HighQueue in shared memory.
        uint8_t version_;
        
        /// @brief Identifity this HighQueue for documentation/diagnostic purposes.
        char name_[32];
        
        /// @brief If true, when the queue is full and no consumer is available messages will be discarded.
        bool discardMessagesIfNoConsumer_;
        
        /// @brief A strategy to control how the producer waits when the queue is full
        WaitStrategy producerWaitStrategy_;
        
        /// @brief a strategy to control how the consumer waits when the queue is empty
        WaitStrategy consumerWaitStrategy_;
        
        /// @brief What is the maximum number of messages this queue can hold?
        size_t entryCount_;
        
        /// @brief offset to the entries that contain the messages: HQEntry[entryCount_]
        ///
        /// This array is indexed by [Position % entryCount_].  It can be thought of as an infinite vector
        /// where only the last entryCount_ entries are visible.
        Offset entries_;
        
        /// @brief Offset to a cacheline containing Position of the next entry to be read.
        Offset readPosition_;

        /// @brief Offset to a cacheline containing Position of the next entry to be published.
        Offset publishPosition_;

        /// @brief Offset to a cacheline containing Position of the next entry to be reserved.
        /// An Position must be reserved before it is published.
        Offset reservePosition_;

        /// @brief Offset to a memory pool used allocate memory for Messages
        /// This is for use when the HighQueeue resides in shared memory meaning the Message buffers
        /// must be in the same shared memmory block as the HighQueue itself.
        ///
        /// For local (intra-process) HighQueues, this will be zero.
        Offset memoryPool_;

        ////////////////////////////////////////////////
        // Dynamically changable values that
        // change relatively infrequently, and therefore
        // do not need to be in separate cache lines.

        /// @brief True when a consumer is attached to the queue.
        /// Used to honor the discardMessagesIfNoConsumer option
        std::atomic<bool> consumerPresent_; // VC12 has a bug in std::atomic_bool so we fall back on the more generic atomic<T>
                                            // http://stackoverflow.com/questions/15750917/initializing-stdatomic-bool

        /// @brief A count of the producers attached to the pool
        /// Mostly for diagnostic purposes.
        std::atomic<uint32_t> producersPresent_;

        ////////////////////////////////////////////
        // TODO: Move These to a separate cache line.  
        /// They change on a per-message basis
        // OTHH if we're using a mutex latency is not the primary concern.
        std::mutex waitMutex_;
        std::condition_variable producerWaitConditionVariable_;
        std::condition_variable consumerWaitConditionVariable_;
        /// @brief true if a producer is waiting on producerWaitConditionVariable_ .. an optimization to avoid unnecessary notifies.
        bool producerWaiting_;
        /// @brief true if a consummer is waiting on consumerWaitConditionVariable_ .. an optimization to avoid unnecessary notifies.
        bool consumerWaiting_;

        /// @brief Initialize the header during construction of a HighQueue
	    HQHeader(
            const std::string & name, 
            HQAllocator & allocator, 
            const CreationParameters & parameters,
            HQMemoryBlockPool * pool = 0);
        void allocateInternalMessages(HQMemoryBlockPool * pool);

        /// @brief Release all messages back to the memory pool during destruction of a HighQueue
        /// For local HighQueues with shared memory pools this makes the memory available for reuse.
        /// For shared memory HighQueues this is not really necessary.
        void releaseInternalMessages();
    };
}
