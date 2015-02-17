// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "RoundRobin.h"

#include <Steps/StepFactory.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<RoundRobin> registerStep("round_robin");
}

RoundRobin::RoundRobin()
    : messagesHandled_(0)
{
}

void RoundRobin::handle(Message & message)
{
    if(!stopping_)
    { 
        LogTrace("RoundRobin copy.");
        send(messagesHandled_ % getDestinationCount(), message);
        ++messagesHandled_;
    }
}

void RoundRobin::finish()
{
    LogStatistics("RoundRobin messages: " << messagesHandled_);
}

