// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "MockMessageAnalyzer.hpp"

#include <Steps/StepFactory.hpp>
#include <Mocks/MockMessage.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<MockMessageAnalyzer> registerStep("test_message_analyzer", "Extract meta information from a test message.");
}

MockMessageAnalyzer::MockMessageAnalyzer()
    : messagesHandled_(0)
{
}

void MockMessageAnalyzer::handle(Message & message)
{
    if(!stopping_)
    {
        auto type = message.getType();
        if(type == Message::MulticastPacket)
        {
            message.setType(Message::MockMessage);
            auto mockMessage = message.get<MockMessage<0> >();
            message.setSequence(mockMessage->getSequence());
        }
        send(message);
        ++messagesHandled_;
    }
}

void MockMessageAnalyzer::finish()
{
    LogStatistics("MockMessageAnalyzer " << name_ << " messages: " << messagesHandled_);
}

