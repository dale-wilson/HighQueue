// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "Leaker.h"
#include <Steps/StepFactory.h>
#include <Steps/SharedResources.h>
#include <Steps/Configuration.h>

#include <HighQueue/MemoryPool.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<Leaker> registerStep("leaker", "**TESTING**: Discard selected messages.  Pass the rest true to the next step.");

    const std::string keyCount = "count";
    const std::string keyEvery = "every";
    const std::string keyHeartbeats = "heartbeats";
    const std::string keyShutdowns = "shutdowns";
}

Leaker::Leaker()
    : count_(0)
    , every_(0)
    , leakHeartbeats_(false)
    , leakShutdowns_(false)
    , messageNumber_(0)
    , published_(0)
    , leaked_(0)
{
}

bool Leaker::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyCount)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            count_ = size_t(value);
            return true;
        }
    }
    if(key == keyEvery)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            every_ = size_t(value);
            return true;
        }
    }
    else if(key == keyHeartbeats)
    {
        if(configuration.getValue(leakHeartbeats_))
        {
            return true;
        }
    }
    else if(key == keyShutdowns)
    {
        if(configuration.getValue(leakShutdowns_))
        {
            return true;
        }
    }
    else
    {
        return Step::configureParameter(key, configuration);
    }
    return false;
}

void Leaker::validate()
{
    mustHaveDestination();
    if(every_ == 0)
    {
        std::stringstream msg;
        msg << "Leaker: \"" << keyEvery << "\" cannot be zero.";
        throw std::runtime_error(msg.str());
    }
    Step::validate();
}

void Leaker::handle(Message & message)
{
    bool leak = true;
    auto type = message.getType();
    if(type == Message::MessageType::Heartbeat)
    {
        leak = leakHeartbeats_;
    }
    else if(type == Message::MessageType::Shutdown)
    {
        leak = leakShutdowns_;
    }
    if(leak)
    {
        ++messageNumber_;
        auto p = messageNumber_ % every_;
        if(p >= count_)
        {
            send(message);
            ++published_;
        }
        else
        {
            ++leaked_;
        }
    }
}

void Leaker::finish()
{
    LogStatistics("Leaker published: " << published_);
    LogStatistics("Leaker leaked: " << leaked_);
}
