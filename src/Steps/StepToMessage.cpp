// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "StepToMessage.h"
#include <Steps/SharedResources.h>

#include <HighQueue/MemoryPool.h>
#include <HighQueue/Message.h>

using namespace HighQueue;
using namespace Steps;

StepToMessage::StepToMessage()
{
}

StepToMessage::~StepToMessage()
{
}

void StepToMessage::configureResources(SharedResources & resources)
{
    resources.requestMessages(1);
    return Step::configureResources(resources);
}

void StepToMessage::attachResources(SharedResources & resources)
{
    auto & pool = resources.getMemoryPool();
    if(pool)
    {
        outMessage_.reset(new Message(pool));
    }
    Step::attachResources(resources);
}

void StepToMessage::validate()
{
    mustHaveDestination();
    if(!outMessage_)
    {
        throw std::runtime_error("StepToMessage: Can't initialize output message. No memory pool attached.");
    }
    Step::validate();
}        
