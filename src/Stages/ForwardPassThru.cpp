// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "ForwardPassThru.h"

#include <StagesSupport/StageFactory.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<ForwardPassThru> registerStage("forward_pass_thru");
}

ForwardPassThru::ForwardPassThru(uint32_t messageCount)
    : messageCount_(messageCount)
    , messagesHandled_(0)
{
    setName("ForwardPassThru"); // default name
}

void ForwardPassThru::handle(Message & message)
{
    if(!stopping_)
    { 
        LogTrace("ForwardPassThru copy.");
        send(message);
        ++messagesHandled_;
        if(messageCount_ != 0 && messagesHandled_ >= messageCount_)
        {
            LogTrace("ForwardPassThru stop: message count: " << messagesHandled_);
            stop();
        }
    }
}

