// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "StepToMessage.hpp"
#include <Steps/SharedResources.hpp>

#include <HighQueue/MemoryPool.hpp>
#include <HighQueue/Message.hpp>

using namespace HighQueue;
using namespace Steps;

StepToMessage::StepToMessage()
    : destinationIsRequired_(true)
{
}

StepToMessage::~StepToMessage()
{
}

void StepToMessage::configureResources(const SharedResourcesPtr & resources)
{
    resources->requestMessages(1);
    return Step::configureResources(resources);
}

void StepToMessage::attachResources(const SharedResourcesPtr & resources)
{
    auto & pool = resources->getMemoryPool();
    if(pool)
    {
        outMessage_.reset(new Message(pool));
    }
    Step::attachResources(resources);
}

void StepToMessage::validate()
{
    if(destinationIsRequired_)
    { 
        mustHaveDestination();
        if(!outMessage_)
        {
            throw std::runtime_error("StepToMessage: Can't initialize output message. No memory pool attached.");
        }
    }
    Step::validate();
}        
