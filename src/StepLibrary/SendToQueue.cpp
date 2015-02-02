// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "SendToQueue.h"
#include <Steps/StepFactory.h>
#include <Steps/Configuration.h>
#include <Steps/BuildResources.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<SendToQueue> registerStep("send_to_queue");

    const std::string keyQueueName = "queue";
}


SendToQueue::SendToQueue()
{
}

bool SendToQueue::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyQueueName)
    {
        configuration.getValue(queueName_);
        return true;
    }
    else
    {
        return Step::configureParameter(key, configuration);
    }
}

void SendToQueue::attachResources(BuildResources & resources)
{
    connection_ = resources.findQueue(queueName_);
    if(connection_)
    {
        connection_->willProduce();
    }
    else
    {
        LogError("SendToQueue can't find queue \"" << queueName_ << "\" in [" << resources.getQueueNames() << "]");

    }
    Step::attachResources(resources);
}


void SendToQueue::validate()
{
    mustNotHaveDestination();
    if(!connection_)
    {
        std::stringstream msg;
        msg << "SendToQueue can't find queue \"" << queueName_ << "\".";
        throw std::runtime_error(msg.str());
    }
    Step::validate();
}

void SendToQueue::start()
{
    producer_.reset(new Producer(connection_));
    Step::start();
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

void SendToQueue::stop()
{
    Step::stop();
    if(producer_)
    {
        producer_->stop();
    }
}