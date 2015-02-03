// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "AsioStepToMessage.h"
#include <Steps/SharedResources.h>
using namespace HighQueue;
using namespace Steps;
AsioStepToMessage::AsioStepToMessage()
{
}

AsioStepToMessage::~AsioStepToMessage()
{
}

void AsioStepToMessage::configureResources(SharedResources & resources)
{
    resources.requestAsioThread(0, 1);
    StepToMessage::configureResources(resources);
}

void AsioStepToMessage::attachResources(SharedResources & resources)
{
    ioService_ = resources.getAsioService();
    StepToMessage::attachResources(resources);
}

void AsioStepToMessage::validate()
{
    if(!ioService_)
    {
        std::stringstream msg;
        msg << "No Asio service attached to " << name_;
        std::runtime_error(msg.str());
    }
}


void AsioStepToMessage::start()
{
    me_ = shared_from_this();
}

void AsioStepToMessage::finish()
{
    StepToMessage::finish();
    if(me_)
    { 
        me_.reset();
    }
}
