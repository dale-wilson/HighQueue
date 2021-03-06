// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "ForwardPassThru.hpp"

#include <Steps/StepFactory.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<ForwardPassThru> registerStep("forward_pass_thru", "Pass the message to the next stage as-is with no copy.");
}

ForwardPassThru::ForwardPassThru()
    : messagesHandled_(0)
{
}

void ForwardPassThru::handle(Message & message)
{
    if(!stopping_)
    { 
        LogTrace("ForwardPassThru copy.");
        send(message);
        ++messagesHandled_;
    }
}

void ForwardPassThru::finish()
{
    LogStatistics("Forward Pass Thru messages: " << messagesHandled_);
}

