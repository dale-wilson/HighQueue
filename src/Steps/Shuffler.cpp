// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StepsSupport/StepPch.h>

#include "Shuffler.h"
#include <StepsSupport/StepFactory.h>
#include <StepsSupport/BuildResources.h>
#include <HighQueue/MemoryPool.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<Shuffler> registerStep("shuffler");

    const std::string keyLookAhead = "look_ahead";
}

Shuffler::Shuffler(size_t lookAhead)
    : lookAhead_(lookAhead)
    , position_(0)
{
    setName("Shuffler"); // default name
}

/// @brief Configure 
/// Lifecycle 2: Configure
bool Shuffler::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    int todo;
    return StepToMessage::configureParameter(key, configuration);
}

void Shuffler::configureResources(BuildResources & resources)
{
    int todo;
    return StepToMessage::configureResources(resources);

}

/// @brief Attach resources
void Shuffler::attachResources(BuildResources & resources)
{
    auto & memoryPool = resources.getMemoryPool();
    if(!memoryPool)
    {
        throw std::runtime_error("OrderedMerge: no memory pool available.");
    }

    while(pendingMessages_.size() < lookAhead_)
    {
        pendingMessages_.emplace_back(memoryPool);
    }
    StepToMessage::attachResources(resources);
}

void Shuffler::validate()
{
    if(pendingMessages_.size() < lookAhead_)
    {
        throw std::runtime_error("Shuffler working messages not initialized. Missing call to attachConnection or attachMemoryPool?");
    }
    StepToMessage::validate();
}

void Shuffler::handle(Message & message)
{
    auto type = message.getType();
    if(type == Message::MessageType::Heartbeat)
    {
        handleHeartbeat(message);
    }
    else if(type == Message::MessageType::Shutdown)
    {
        handleShutdown(message);
    }
    else
    {
        handleDataMessage(message);
    }
}

void Shuffler::handleHeartbeat(Message & message)
{
    publishPendingMessages();
}

void Shuffler::handleShutdown(Message & message)
{
    // make the user call stop();
    handleDataMessage(message);
    publishPendingMessages();
}

void Shuffler::handleDataMessage(Message & message)
{
    position_ += relativelyPrime_;
    size_t index = position_ % lookAhead_;
    if(!pendingMessages_[index].isEmpty())
    {
        send(pendingMessages_[index]);
    }
    message.moveTo(pendingMessages_[index]);
}

void Shuffler::publishPendingMessages()
{
    for(auto & message : pendingMessages_)
    {
        if(!message.isEmpty())
        {
            send(message);
        }
    }
}
