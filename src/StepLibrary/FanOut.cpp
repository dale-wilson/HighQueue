// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "FanOut.h"

#include <Steps/StepFactory.h>
#include <Steps/SharedResources.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<FanOut> registerStep("fan_out", "Distribute identical copies of messages to all destinations.");
}

FanOut::FanOut()
    : messagesHandled_(0)
    , messagesSent_(0)
{
}

void FanOut::handle(Message & message)
{
    if(!stopping_)
    { 
        for(size_t nDestination = 0; nDestination < getDestinationCount(); ++nDestination)
        {
            outMessage_->appendBinaryCopy(message.get(), message.getUsed());
            message.copyMetaInfoTo(*outMessage_);
            send(nDestination, *outMessage_);
            ++messagesSent_;
        }
        ++messagesHandled_;
    }
}

void FanOut::finish()
{
    LogStatistics("FanOut " << name_ <<" messages received: " << messagesHandled_);
    LogStatistics("FanOut " << name_ <<" messages sent: " << messagesSent_);
}

