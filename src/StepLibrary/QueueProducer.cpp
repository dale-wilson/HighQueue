// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "QueueProducer.h"
#include <Steps/StepFactory.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<QueueProducer> registerStep("queue_producer");
}


QueueProducer::QueueProducer()
    : solo_(false)
{
    setName("QueueProducer"); // default name
}

void QueueProducer::configureSolo(bool solo)
{
    solo_ = solo;
}

bool QueueProducer::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    int todo;
    return Step::configureParameter(key, configuration);
}

void QueueProducer::configureResources(BuildResources & resources)
{
    return Step::configureResources(resources);
}



void QueueProducer::handle(Message & message)
{
    auto type = message.getType();
    producer_->publish(message);
    if(type == Message::MessageType::Shutdown)
    {
        stop();
    }
}

void QueueProducer::attachConnection(const ConnectionPtr & connection)
{
    connection_ = connection;
    producer_.reset(new Producer(connection_, solo_));
}

void QueueProducer::validate()
{
    mustNotHaveDestination();
    if(!connection_)
    {
        throw std::runtime_error("QueueProducer must have an attached Connection");
    }
}
