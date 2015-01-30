// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "BinaryPassThru.h"
#include <StagesSupport/StageFactory.h>
using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<BinaryPassThru> registerStage("binary_copy");
}


BinaryPassThru::BinaryPassThru(uint32_t messageCount)
    : messageCount_(messageCount)
    , messagesHandled_(0)
{
    setName("BinaryPassThru"); // default name
}
        
void BinaryPassThru::handle(Message & message)
{
    if(!stopping_)
    { 
        LogDebug("BinaryPassThru copy.");
        outMessage_->appendBinaryCopy(message.get(), message.getUsed());
        message.moveMetaInfoTo(*outMessage_);
        send(*outMessage_);
        auto type = message.getType();
        if(type == Message::MessageType::Shutdown)
        {
            stop();
        }
        ++messagesHandled_;
        if(messageCount_ != 0 && messagesHandled_ >= messageCount_)
        {
            stop();
        }
    }
}
