/// @file MemoryPool.h
// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <HighQueue/details/HQMemoryBlockPool.hpp>

namespace HighQueue
{
    class HighQueue_Export MemoryPool;
    typedef std::shared_ptr<MemoryPool> MemoryPoolPtr;
}
