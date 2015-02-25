// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "AsioStep.hpp"
#include <Steps/SharedResources.hpp>
using namespace HighQueue;
using namespace Steps;
AsioStep::AsioStep()
{
}

AsioStep::~AsioStep()
{
}

void AsioStep::configureResources(SharedResources & resources)
{
    resources.requestAsioThread(0, 1);
    Step::configureResources(resources);
}

void AsioStep::attachResources(SharedResources & resources)
{
    ioService_ = resources.getAsioService();
    Step::attachResources(resources);
}

void AsioStep::validate()
{
    if(!ioService_)
    {
        std::stringstream msg;
        msg << "No Asio service attached to " << name_;
        std::runtime_error(msg.str());
    }
}


void AsioStep::start()
{
    me_ = shared_from_this();
}

void AsioStep::finish()
{
    Step::finish();
    if(me_)
    { 
        me_.reset();
    }
}
