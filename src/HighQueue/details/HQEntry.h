/// @file HighQEntry.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/details/HQDefinitions.h>
#include <HighQueue/details/HQAllocator.h>
#include <HighQueue/Message.h>

namespace HighQueue
{
	PRE_CACHE_ALIGN
	struct HighQEntry
	{
        enum Status : uint8_t
        {
            OK,
            EMPTY,
            SKIP
        };
        Message message_;
        Status status_;

        HighQEntry()
            : status_(Status::EMPTY)
        {
        }

        static size_t alignedSize()
        {
            return HQAllocator::align(sizeof(HighQEntry), CacheLineSize);
        }

	} POST_CACHE_ALIGN;
}
