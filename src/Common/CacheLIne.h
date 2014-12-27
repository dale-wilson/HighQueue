// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

namespace MPass
{
    /// @brief A cache aligned array of bytes.
    PRE_CACHE_ALIGN
    struct CacheLine
    {
        byte_t bytes_[CacheLineSize];
    }POST_CACHE_ALIGN;
}