#pragma once

namespace MPass
{
    PRE_CACHE_ALIGN
    struct CacheLine
    {
        byte_t bytes_[CacheLineSize];
    }POST_CACHE_ALIGN;
}