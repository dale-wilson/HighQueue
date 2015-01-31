// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "StageToMessage.h"
#include <StagesSupport/BuildResources.h>

#include <HighQueue/MemoryPool.h>
#include <HighQueue/Message.h>

using namespace HighQueue;
using namespace Stages;

StageToMessage::StageToMessage()
{
}

StageToMessage::~StageToMessage()
{
}

void StageToMessage::configureResources(BuildResources & resources)
{
    resources.requestMessages(1);
    return Stage::configureResources(resources);
}

void StageToMessage::attachResources(BuildResources & resources)
{
    auto & pool = resources.getMemoryPool();
    if(pool)
    {
        outMessage_.reset(new Message(pool));
    }
    Stage::attachResources(resources);
}

void StageToMessage::validate()
{
    mustHaveDestination();
    if(!outMessage_)
    {
        throw std::runtime_error("StageToMessage: Can't initialize output message. No memory pool attached.");
    }
}        
