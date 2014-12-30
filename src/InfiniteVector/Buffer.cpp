// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/MPassPch.h>

#include "Buffer.h"
#include <InfiniteVector/MemoryBlockPool.h>

using namespace MPass;
using namespace InfiniteVector;


Buffer::Buffer()
: container_(0)
, capacity_(0)
, offset_(0)
, used_(0)
, offsetSplit_(0)
, usedSplit_(0)
, type_(Invalid)
{
}

Buffer::~Buffer()
{
    try{
        release();
    }
    catch(...) 
    {
        //ignore this (sorry!)
    }
}

void Buffer::set(MemoryBlockPool * container, size_t capacity, size_t offset, size_t used)
{
    container_ = reinterpret_cast<byte_t *>(container);
    capacity_ = (capacity == 0) ? used : capacity;
    offset_ = offset;
    used_ = used;
    offsetSplit_ = 0;
    usedSplit_ = 0;
    type_ = Normal;
}

byte_t * Buffer::getContainer()const
{
    mustBeValid();
    return container_;
}

size_t Buffer::getOffset()const
{
    return offset_;
}

void Buffer::release()
{
    if(type_ == Normal)
    {
        auto pool = reinterpret_cast<MemoryBlockPool *>(container_);
        pool->release(*this);
    }
    else
    {
        reset();
    }
}

inline void Buffer::reset()
{
    container_ = 0;
    capacity_ = 0;
    offset_ = 0;
    used_ = 0;
    offsetSplit_ = 0;
    usedSplit_ = 0;
    type_ = Invalid;
}
