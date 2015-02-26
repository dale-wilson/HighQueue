// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "MockMessageConsumer.hpp"
#include <Steps/StepFactory.hpp>
using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<MockMessageConsumer<SmallMockMessage> > registerStepSmall("small_test_message_consumer", "Validate and consume small test messages");
    StepFactory::Registrar<MockMessageConsumer<MediumMockMessage> > registerStepMedium("medium_test_message_consumer", "Validate and consume medium test messages");
    StepFactory::Registrar<MockMessageConsumer<LargeMockMessage> > registerStepLarge("large_test_message_consumer", "Validate and consume large test messages");

    const std::string keyMessageCount = "message_count";


}

BaseMessageConsumer::BaseMessageConsumer()
    : messageCount_(0)
    , heartbeats_(0)
    , shutdowns_(0)
    , gaps_(0)
    , messagesHandled_(0)
    , nextSequence_(0)
    , sequenceError_(0)
    , unexpectedMessageError_(0)
{
}

BaseMessageConsumer::~BaseMessageConsumer()
{
}

bool BaseMessageConsumer::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyMessageCount)
    {
        uint64_t messageCount;
        if(!configuration.getValue(messageCount))
        {
            LogFatal("MessageConsumer can't interpret value for " << keyMessageCount);
        }
        messageCount_ = uint32_t(messageCount);
        return true;
    }
    return Step::configureParameter(key, configuration);
}

void BaseMessageConsumer::logStats()
{
    LogStatistics("MessageConsumer " << name_ << " heartbeats:" << heartbeats_);
    LogStatistics("MessageConsumer " << name_ << " shutdowns: " << shutdowns_);
    LogStatistics("MessageConsumer " << name_ << " messagesHandled:" << messagesHandled_);
    LogStatistics("MessageConsumer " << name_ << " sequenceError:" << sequenceError_);
    LogStatistics("MessageConsumer " << name_ << " unexpectedMessageError:" << unexpectedMessageError_);
    Step::finish();
}

