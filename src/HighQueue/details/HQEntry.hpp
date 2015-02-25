// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/details/HQDefinitions.hpp>
#include <HighQueue/details/HQAllocator.hpp>
#include <HighQueue/Message.hpp>

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

        template <typename Allocator>
        HighQEntry(Allocator & allocator)
            : message_(allocator)
            , status_(Status::EMPTY)
        {
        }

        static size_t alignedSize()
        {
            return HQAllocator::align(sizeof(HighQEntry), CacheLineSize);
        }

    } POST_CACHE_ALIGN;
}
