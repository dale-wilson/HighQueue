// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "SendToQueue.h"
#include <Steps/StepFactory.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<SendToQueue> registerStep("send_to_queue");
}


SendToQueue::SendToQueue()
    : solo_(false)
{
    setName("SendToQueue"); // default name
}

void SendToQueue::configureSolo(bool solo)
{
    solo_ = solo;
}

bool SendToQueue::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    int todo;
    return Step::configureParameter(key, configuration);
}

void SendToQueue::configureResources(BuildResources & resources)
{
    return Step::configureResources(resources);
}



void SendToQueue::handle(Message & message)
{
    auto type = message.getType();
    producer_->publish(message);
    if(type == Message::MessageType::Shutdown)
    {
        stop();
    }
}

void SendToQueue::attachConnection(const ConnectionPtr & connection)
{
    connection_ = connection;
    producer_.reset(new Producer(connection_, solo_));
}

void SendToQueue::validate()
{
    mustNotHaveDestination();
    if(!connection_)
    {
        throw std::runtime_error("SendToQueue must have an attached Connection");
    }
}
