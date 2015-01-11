// Copyright (c) 2014 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Common/HighQueuePch.h>

#include "Message.h"
#include <HighQueue/details/HQMemoryBlockPool.h>

using namespace HighQueue;

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

void Message::set(HQMemoryBlockPool * container, size_t capacity, size_t offset, size_t used)
{
    container_ = reinterpret_cast<byte_t *>(container);
    capacity_ = (capacity == 0) ? used : capacity;
    offset_ = offset;
    used_ = used;
}

byte_t * Message::getContainer()const
{
    return container_;
}

size_t Message::getOffset()const
{
    return offset_;
}

void Message::release()
{
    if(container_ != 0)
    {
        auto pool = reinterpret_cast<HQMemoryBlockPool *>(container_);
        pool->release(*this);
    }
}

void Message::reset()
{
    container_ = 0;
    capacity_ = 0;
    offset_ = 0;
    used_ = 0;
}
