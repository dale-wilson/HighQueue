// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <ComponentCommon/ComponentPch.h>

#include "StageToMessage.h"
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Message.h>

using namespace HighQueue;
using namespace Components;

StageToMessage::StageToMessage()
{
}

StageToMessage::~StageToMessage()
{
}

void StageToMessage::attachMemoryPool(const MemoryPoolPtr & pool)
{
    outMessage_.reset(new Message(pool));
}

void StageToMessage::validate()
{
    if(!outMessage_)
    {
        throw std::runtime_error("StageToMessage: Can't initialize output message. No memory pool attached.");
    }
}        
