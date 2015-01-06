/// @file HSQHeader.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Common/HSQueue_Export.h>
#include <HSQueue/details/HSQDefinitions.h>
#include <HSQueue/CreationParameters.h>
#include <HSQueue/details/HSQAllocator.h>

namespace HSQueue
{
    struct HSQueue_Export HSQHeader
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

	    HSQHeader(
            const std::string & name, 
            HSQAllocator & allocator, 
            const CreationParameters & parameters);
        void allocateInternalMessages();
        void releaseInternalMessages();
    };
}
