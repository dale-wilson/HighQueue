/// @file HQHeader.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HighQueue_Export.h>
#include <HighQueue/details/HQDefinitions.h>
#include <HighQueue/CreationParameters.h>
#include <HighQueue/details/HQAllocator.h>

namespace HighQueue
{
    struct HighQueue_Export HQHeader
    {
        Signature signature_;
        uint8_t version_;
        char name_[32];
        size_t entryCount_;
        Offset entries_;
        Offset readPosition_;
        Offset publishPosition_;
        Offset reservePosition_;
        Offset memoryPool_;

        WaitStrategy producerWaitStrategy_;
        WaitStrategy consumerWaitStrategy_;
        std::mutex waitMutex_;
        std::condition_variable producerWaitConditionVariable_;
        std::condition_variable consumerWaitConditionVariable_;
        bool discardIfNoConsumer_;
        bool consumerWaiting_;
        bool producerWaiting_;
        std::atomic_bool consumerPresent_;
        std::atomic<uint32_t> producersPresent_;

	    HQHeader(
            const std::string & name, 
            HQAllocator & allocator, 
            const CreationParameters & parameters,
            HQMemoryBlockPool * pool = 0);
        void allocateInternalMessages(HQMemoryBlockPool * pool);
        void releaseInternalMessages();
    };
}
