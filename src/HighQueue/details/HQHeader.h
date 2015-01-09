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

        ConsumerWaitStrategy consumerWaitStrategy_;
        std::mutex consumerWaitMutex_;
        std::condition_variable consumerWaitConditionVariable_;

	    HQHeader(
            const std::string & name, 
            HQAllocator & allocator, 
            const CreationParameters & parameters,
            HQMemoryBLockPool * pool = 0);
        void allocateInternalMessages(HQMemoryBLockPool * pool);
        void releaseInternalMessages();
    };
}
