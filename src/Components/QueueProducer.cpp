// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StageCommon/StagePch.h>

#include "QueueProducer.h"

using namespace HighQueue;
using namespace Stages;

QueueProducer::QueueProducer()
{
}

void QueueProducer::handle(Message & message)
{
    auto type = message.meta().type_;
    producer_->publish(message);
    if(type == Message::Meta::MessageType::Shutdown)
    {
        stop();
    }

}

void QueueProducer::attachConnection(const ConnectionPtr & connection)
{
    connection_ = connection;
    producer_.reset(new Producer(connection_));
}

void QueueProducer::validate()
{
    mustNotHaveDestination();
    if(!connection_)
    {
        throw std::runtime_error("QueueProducer must have an attached Connection");
    }
}
