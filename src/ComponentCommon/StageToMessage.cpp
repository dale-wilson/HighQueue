// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <ComponentCommon/ComponentPch.h>

#include "StageToMessage.h"
#include <HighQueue/MemoryPool.h>
#include <HighQueue/Message.h>
#include <HighQueue/Connection.h>

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
    Stage::attachMemoryPool(pool);
}

void StageToMessage::attachConnection(const ConnectionPtr & connection)
{
    outMessage_.reset(new Message(connection));
    Stage::attachConnection(connection);
}

void StageToMessage::validate()
{
    mustHaveDestination();
    if(!outMessage_)
    {
        throw std::runtime_error("StageToMessage: Can't initialize output message. No memory pool attached.");
    }
}        
