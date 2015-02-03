// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "BinaryPassThru.h"
#include <Steps/StepFactory.h>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<BinaryPassThru> registerStep("binary_copy");
}


BinaryPassThru::BinaryPassThru()
    : messagesHandled_(0)
{
}
        
void BinaryPassThru::handle(Message & message)
{
    if(!stopping_)
    { 
        LogDebug("BinaryPassThru copy.");
        outMessage_->appendBinaryCopy(message.get(), message.getUsed());
        message.moveMetaInfoTo(*outMessage_);
        send(*outMessage_);
        ++messagesHandled_;
    }
}

void BinaryPassThru::finish()
{
    LogStatistics("Binary Pass Thru messages: " << messagesHandled_);
}
