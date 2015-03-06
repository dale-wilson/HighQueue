// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "Stopper.hpp"
#include <Steps/StepFactory.hpp>
#include <Steps/SharedResources.hpp>
#include <Steps/Configuration.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<Stopper> registerStep("stopper", "Shut the system down.");

    const std::string keyHeartbeats = "heartbeats";
    const std::string keyShutdowns = "shutdowns";
    const std::string keyDataMessages = "data_messages";
}

Stopper::Stopper()
    : messagesExpected_(0)
    , heartbeatsExpected_(0)
    , shutdownsExpected_(0)
    , messagesHandled_(0)
    , heartbeatsHandled_(0)
    , shutdownsHandled_(0)
{
}

bool Stopper::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyDataMessages)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            messagesExpected_ = uint32_t(value);
            return true;
        }
    }
    else if(key == keyHeartbeats)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            heartbeatsExpected_ = uint32_t(value);
            return true;
        }
    }
    else if(key == keyShutdowns)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            shutdownsExpected_ = uint32_t(value);
            return true;
        }
    }
    else
    {
        return Step::configureParameter(key, configuration);
    }
    return false;
}

std::ostream & Stopper::usage(std::ostream & out) const
{
    out << keyHeartbeats << ": Stop after this many heartbeats." << std::endl;
    out << keyShutdowns << ": Stop after this many shutdown." << std::endl;
    out << keyDataMessages << ": Stop after this many normal messages (non-heartbeat, non-shutdown)." << std::endl;
    return Step::usage(out);
}

void Stopper::attachResources(const SharedResourcesPtr & resources)
{
    resources_ = resources;
    Step::attachResources(resources);
}

void Stopper::handle(Message & message)
{
    if(!stopping_)
    {
        auto type = message.getType();
        switch(type)
        {
            case Message::MessageType::Heartbeat:
            {
                ++heartbeatsHandled_;
                LogVerbose("Stopper: heartbeats: " << heartbeatsHandled_ << '/' << heartbeatsExpected_);
                if(heartbeatsExpected_ > 0 && heartbeatsHandled_ >= heartbeatsExpected_ && resources_)
                {
                    LogInfo("Stopper: heartbeats: " << heartbeatsHandled_ << '/' << heartbeatsExpected_);
                    resources_->stop();
                    resources_.reset();
                }
                break;
            }
            case Message::MessageType::Shutdown:
            {
                ++shutdownsHandled_;
                LogVerbose("Stopper: shutdowns: " << shutdownsHandled_ << '/' << shutdownsExpected_);
                if(shutdownsExpected_ > 0 && shutdownsHandled_ >= shutdownsExpected_ && resources_)
                {
                    LogInfo("Stopper: shutdowns: " << shutdownsHandled_ << '/' << shutdownsExpected_);
                    resources_->stop();
                    resources_.reset();
                }
                break;
            }
            default:
            {
                ++messagesHandled_;
                LogVerbose("Stopper: messages: " << messagesHandled_ << '/' << messagesExpected_);
                if(messagesExpected_ > 0 && messagesHandled_ >= messagesExpected_ && resources_)
                {
                    LogInfo("Stopper: messages: " << messagesHandled_ << '/' << messagesExpected_);
                    resources_->stop();
                    resources_.reset();
                }
                break;
            }
        }
    }
    send(message);
}
