// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "HeartbeatProducer.h"
#include <StagesSupport/StageFactory.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<HeartbeatProducer> registerStage("heartbeat");
}


HeartbeatProducer::HeartbeatProducer(std::chrono::milliseconds interval)
    : interval_(interval.count())
    , cancel_(false)
{
    setName("HeartbeatProducer"); // default name
}

void HeartbeatProducer::start()
{
    timer_.reset(new Timer(*ioService_));
    startTimer();
}

void HeartbeatProducer::stop()
{
    LogTrace("***HeartbeatProducer Cancel Timer");
    cancel_ = true;
    timer_->cancel();
    LogTrace("***HeartbeatProducer Stopping");
    AsioStageToMessage::stop();
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
        auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        outMessage_->setTimestamp(timestamp);
        outMessage_->appendBinaryCopy(&timestamp, sizeof(timestamp));
        LogTrace("Publish Heartbeat: " << timestamp);
        send(*outMessage_);
    }
    startTimer();
}

