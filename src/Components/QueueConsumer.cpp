// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>

#include "QueueConsumer.h"

using namespace HighQueue;
using namespace Stages;

QueueConsumer::QueueConsumer()
:stopOnShutdownMessage_(false)
{
}

void QueueConsumer::handle(Message & message)
{
    throw std::runtime_error("QueueConsuemr does not accept incoming Messages");
}

void QueueConsumer::attachConnection(const ConnectionPtr & connection)
{
    connection_ = connection;
    consumer_.reset(new Consumer(connection_));
    message_.reset(new Message(connection_));
}

void QueueConsumer::validate()
{
    mustHaveDestination();
    if(!connection_)
    {
        throw std::runtime_error("QueueConsumer must have an attached Connection");
    }
}

void QueueConsumer::setStopOnShutdownMessage(bool value)
{
    stopOnShutdownMessage_ = value;
}

void QueueConsumer::start()
{
    me_ = shared_from_this();
    thread_ = std::thread(std::bind(
        &QueueConsumer::startThread,
        this));
}

void QueueConsumer::startThread()
{
    LogTrace("QueueConsumer thread starts. @" << (void *) this);
    run();
    LogTrace("QueueConsumer thread exits. @" << (void *) this);
}

void QueueConsumer::stop()
{
    Stage::stop();
    consumer_->stop();
}
void QueueConsumer::finish()
{
    if(me_)
    {
        if(std::this_thread::get_id() != thread_.get_id())
        {
            thread_.join();
            me_.reset();
        }
        else
        {
            LogWarning("QueueConsumer::finish() should not be called from the running thread.");
        }
    }
}


void QueueConsumer::run()
{
    while(!stopping_)
    {
        stopping_ = !consumer_->getNext(*message_);
        if(!stopping_)
        {
            auto type = message_->meta().type_;
            send(*message_);
            if(stopOnShutdownMessage_ && type == Message::Meta::MessageType::Shutdown)
            {
                stop();
            }
            
        }
        else
        {
            LogTrace("QueueConsumer::stopped by getNext? " << (void *) this << ": " << stopping_);
        }
    }
}

