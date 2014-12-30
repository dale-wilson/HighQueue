/// @file IvHeader.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "IvDefinitions.h"
#include <InfiniteVector/CreationParameters.h>
#include <InfiniteVector/details/IvAllocator.h>

namespace MPass
{
namespace InfiniteVector
{
    struct IvHeader
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

	    IvHeader(
            const std::string & name, 
            IvAllocator & allocator, 
            const CreationParameters & parameters);
        void allocateInternalMessages();
        void releaseInternalMessages();
    };
}
}