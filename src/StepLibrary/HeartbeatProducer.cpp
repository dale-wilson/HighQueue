3// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "HeartbeatProducer.h"
#include <Steps/StepFactory.h>
#include <Steps/Configuration.h>
#include <Steps/BuildResources.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<HeartbeatProducer> registerStep("heartbeat");

    const std::string keyInterval = "milliseconds";

    //TODO: Make this public
    struct HeartbeatMessage
    {
        std::chrono::milliseconds timeStamp_;
        HeartbeatMessage(uint64_t now)
        {
            timeStamp_ = std::chrono::milliseconds(now);
        }
    };
}

HeartbeatProducer::HeartbeatProducer()
    : interval_(1000)
    , cancel_(false)
{
}

void HeartbeatProducer::setInterval(std::chrono::milliseconds interval)
{
    interval_ = Interval(interval.count());
}

bool HeartbeatProducer::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyInterval)
    {
        uint64_t msec;
        if(!configuration.getValue(msec))
        {
            LogError("Can't interpret parameter \"" << key << "\" for " << name_);
            return false;
        }
        interval_ = boost::posix_time::milliseconds(msec);
        return true;
    }
    else
    {
        return AsioStepToMessage::configureParameter(key, configuration);
    }
}

void HeartbeatProducer::configureResources(BuildResources & resources)
{
    resources.requestMessageSize(sizeof(HeartbeatMessage));
    AsioStepToMessage::configureResources(resources);
}


void HeartbeatProducer::start()
{
    timer_.reset(new Timer(*ioService_));
    LogTrace("Heartbeat: StartTimer " << interval_.total_milliseconds());
    startTimer();
}

void HeartbeatProducer::stop()
{
    LogTrace("HeartbeatProducer Stopping:  Cancel Timer");
    cancel_ = true;
    timer_->cancel();
    AsioStepToMessage::stop();
}

void HeartbeatProducer::startTimer()
{
    if(!stopping_)
    {
        timer_->expires_from_now(interval_);
        timer_->async_wait(boost::bind(
            &HeartbeatProducer::handleTimer, this, boost::asio::placeholders::error));
    }
}

void HeartbeatProducer::handleTimer(const boost::system::error_code& error)
{
    if(error)
    {
        LogTrace("HeartbeatProducer: timer canceled.");
        if(!cancel_) // did we expect this?
        {
            LogError("Error in HeartbeatProducer " << error);
        }
    }
    else if(!paused_)
    {
        outMessage_->setType(Message::Heartbeat);
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        auto timestamp = now.count();
        outMessage_->setTimestamp(timestamp);
        outMessage_->emplace<HeartbeatMessage>(timestamp);
        LogTrace("Publish Heartbeat: " << timestamp);
        send(*outMessage_);
    }
    startTimer();
}

