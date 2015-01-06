/// @file HSQEntry.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HSQueue/details/HSQDefinitions.h>
#include <HSQueue/details/HSQAllocator.h>
#include <HSQueue/Message.h>

namespace HSQueue
{
	PRE_CACHE_ALIGN
	struct HSQEntry
	{
        enum Status : uint8_t
        {
            OK,
            EMPTY,
            SKIP
        };
        HSQueue::Message message_;
        Status status_;

        HSQEntry()
            : status_(Status::EMPTY)
        {
        }

        static size_t alignedSize()
        {
            return HSQAllocator::align(sizeof(HSQEntry), CacheLineSize);
        }

	} POST_CACHE_ALIGN;
}
