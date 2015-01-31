// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StepsSupport/StepPch.h>

#include "AsioStepToMessage.h"
#include <StepsSupport/BuildResources.h>
using namespace HighQueue;
using namespace Steps;
AsioStepToMessage::AsioStepToMessage()
{
}

AsioStepToMessage::~AsioStepToMessage()
{
}

void AsioStepToMessage::attachResources(BuildResources & resources)
{
    ioService_ = resources.getAsioService();
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
