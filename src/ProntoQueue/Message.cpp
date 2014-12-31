// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/MPassPch.h>

#include "Message.h"
#include <ProntoQueue/details/MemoryBlockPool.h>

using namespace MPass;
using namespace ProntoQueue;


Message::Message()
: container_(0)
, capacity_(0)
, offset_(0)
, used_(0)
, offsetSplit_(0)
, usedSplit_(0)
, type_(Invalid)
{
}

Message::~Message()
{
    try{
        release();
    }
    catch(...) 
    {
        //ignore this (sorry!)
    }
}

void Message::set(MemoryBlockPool * container, size_t capacity, size_t offset, size_t used)
{
    container_ = reinterpret_cast<byte_t *>(container);
    capacity_ = (capacity == 0) ? used : capacity;
    offset_ = offset;
    used_ = used;
    offsetSplit_ = 0;
    usedSplit_ = 0;
    type_ = Normal;
}

byte_t * Message::getContainer()const
{
    mustBeValid();
    return container_;
}

size_t Message::getOffset()const
{
    return offset_;
}

void Message::release()
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

inline void Message::reset()
{
    container_ = 0;
    capacity_ = 0;
    offset_ = 0;
    used_ = 0;
    offsetSplit_ = 0;
    usedSplit_ = 0;
    type_ = Invalid;
}
