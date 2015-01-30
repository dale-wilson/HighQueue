// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>

#include "Shuffler.h"
#include <StagesSupport/StageFactory.h>
#include <HighQueue/Connection.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    Registrar<Shuffler> registerStage("shuffler");
}

Shuffler::Shuffler(size_t lookAhead)
    : lookAhead_(lookAhead)
    , position_(0)
{
    setName("Shuffler"); // default name
}

void Shuffler::attachConnection(const ConnectionPtr & connection)
{      
    while(pendingMessages_.size() < lookAhead_)
    {
        pendingMessages_.emplace_back(connection);
    }
    StageToMessage::attachConnection(connection);
}

void Shuffler::attachMemoryPool(const MemoryPoolPtr & memoryPool)
{
    while(pendingMessages_.size() < lookAhead_)
    {
        pendingMessages_.emplace_back(memoryPool);
    }
    StageToMessage::attachMemoryPool(memoryPool);
}

void Shuffler::validate()
{
    if(pendingMessages_.size() < lookAhead_)
    {
        throw std::runtime_error("Shuffler working messages not initialized. Missing call to attachConnection or attachMemoryPool?");
    }
    StageToMessage::validate();
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
