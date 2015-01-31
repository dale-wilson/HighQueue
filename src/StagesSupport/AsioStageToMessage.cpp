// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "AsioStageToMessage.h"
#include <StagesSupport/BuildResources.h>
using namespace HighQueue;
using namespace Stages;
AsioStageToMessage::AsioStageToMessage()
{
}

AsioStageToMessage::~AsioStageToMessage()
{
}

void AsioStageToMessage::attachResources(BuildResources & resources)
{
    ioService_ = resources.getAsioService();
}

void AsioStageToMessage::validate()
{
    if(!ioService_)
    {
        std::stringstream msg;
        msg << "No Asio service attached to " << name_;
        std::runtime_error(msg.str());
    }
}


void AsioStageToMessage::start()
{
    me_ = shared_from_this();
}

void AsioStageToMessage::finish()
{
    StageToMessage::finish();
    if(me_)
    { 
        me_.reset();
    }
}
