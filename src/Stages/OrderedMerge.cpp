// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <StagesSupport/StagePch.h>
#ifdef DISABLED

#include "OrderedMerge.h"
#include <StagesSupport/StageFactory.h>
#include <StagesSupport/BuildResources.h>
#include <StagesSupport/BuildResources.h>
#include <HighQueue/MemoryPool.h>

using namespace HighQueue;
using namespace Stages;

namespace
{
    StageFactory::Registrar<OrderedMerge> registerStageSmall("ordered_merge");
}

OrderedMerge::OrderedMerge()
    : lookAhead_(0)
    , expectedShutdowns_(0)// <-- todo make it stop
    , actualShutdowns_(0)
    , expectedSequenceNumber_(0)
    , lastHeartbeatSequenceNumber_(0)
    , statReceived_(0)
    , statHeartbeats_(0)
    , statShutdowns_(0)
    , statData_(0)
    , statHeartbeatWithoutPublish_(0)
    , statShutdownPublishedGap_(0)
    , statArrivedInOrder_(0)
    , statDuplicatesPrevious_(0)
    , statStashed_(0)
    , statDuplicatesStash_(0)
    , statFuture_(0)
{
    setName("OrderedMerge"); // default name
}

void OrderedMerge::attachResources(BuildResources & resources)
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

    StageToMessage::attachResources(resources);
}

void OrderedMerge::handle(Message & message)
{
    ++statReceived_;
    auto type = message.getType();
    if(type == Message::MessageType::Heartbeat)
    {
        ++statHeartbeats_;
        handleHeartbeat(message);
    }
    else if(type == Message::MessageType::Shutdown)
    {
        ++statShutdowns_;
        handleShutdown(message);
    }
    else
    {
        ++statData_;
        handleDataMessage(message);
    }
}

void OrderedMerge::handleHeartbeat(Message & message)
{
    // todo: we might want to skip some heartbeats depending on frequency
    if(expectedSequenceNumber_ == lastHeartbeatSequenceNumber_)
    {
        ++statHeartbeatWithoutPublish_;
        findAndPublishGap();
        publishPendingMessages();
    }
    lastHeartbeatSequenceNumber_ = expectedSequenceNumber_;
}

void OrderedMerge::handleShutdown(Message & message)
{
    ++actualShutdowns_;
    LogTrace("OrderedMerge received shutdown " << actualShutdowns_ << " of " << expectedShutdowns_);
    if(actualShutdowns_ == expectedShutdowns_)
    {
        while(findAndPublishGap())
        {
            ++statShutdownPublishedGap_;
            publishPendingMessages();
        }
        // forward the shutdown message
        send(message);
        stop();
    }
}

void OrderedMerge::handleDataMessage(Message & message)
{
    auto sequence = message.getSequence();
    if(expectedSequenceNumber_ == 0)
    {
        expectedSequenceNumber_ = sequence;
    }
    LogVerbose("OrderedMerge sequence :" << sequence << " expected " << expectedSequenceNumber_);
    if(sequence == expectedSequenceNumber_)
    {
        ++statArrivedInOrder_;
        LogDebug("OrderedMerge Publish " << sequence);
        send(message);
        ++expectedSequenceNumber_;
        publishPendingMessages();
    }
    else if(sequence < expectedSequenceNumber_)
    {
        ++statDuplicatesPrevious_;
        LogDebug("OrderedMerge Previous Duplicate " << sequence);
        // ignore this one.  We've already seen it.
    }
    else if(sequence - expectedSequenceNumber_ < lookAhead_)
    {
        auto index = sequence % lookAhead_;
        if(pendingMessages_[index].isEmpty())
        {
            ++statStashed_;
            LogDebug("OrderedMerge Stash" << sequence);
            message.moveTo(pendingMessages_[index]);
        }
        else
        {
            LogDebug("OrderedMerge Duplicates Stash " << sequence);
            // ignore this one we are already holding a copy.
        }
    }
    else // Sequence number is beyond the look-ahead window size
    {
        ++statFuture_;
        while (sequence - expectedSequenceNumber_ < lookAhead_)
        {
            if (findAndPublishGap())
            {
                publishPendingMessages();
            }
            else
            {
                publishGapMessage(expectedSequenceNumber_, sequence);
                send(message);
            }
        }
        LogDebug("OrderedMerge Stash future " << sequence);
        auto index = sequence % lookAhead_;
        message.moveTo(pendingMessages_[index]);
    }
}

bool OrderedMerge::findAndPublishGap()
{
    auto gapStart = expectedSequenceNumber_;
    auto gapEnd = gapStart + 1; // the first message that's NOT in the gap
    auto endLookahead = gapStart + lookAhead_;
    while(gapEnd < endLookahead)
    {
        auto index = gapEnd % lookAhead_;
        if(!pendingMessages_[index].isEmpty())
        {
            publishGapMessage(gapStart, gapEnd);
            return true;
        }
        ++gapEnd;
    }
    return false;
}

void OrderedMerge::publishGapMessage(uint32_t gapStart, uint32_t gapEnd)
{
    outMessage_->setType(Message::Gap);
    outMessage_->emplace<GapMessage>(gapStart, gapEnd - 1);
    expectedSequenceNumber_ = gapEnd;
}

void OrderedMerge::publishPendingMessages()
{
    auto index = expectedSequenceNumber_ % lookAhead_;
    while(!pendingMessages_[index].isEmpty())
    {
        LogDebug("OrderedMerge Publish from stash " << expectedSequenceNumber_ );
        send(pendingMessages_[index]);
        ++expectedSequenceNumber_;
        index = expectedSequenceNumber_ % lookAhead_;
    }
}

std::ostream & OrderedMerge::writeStats(std::ostream & out)
{
    return out
        << " Received: " << statReceived_
        << " Heartbeats: " << statHeartbeats_
        << " Shutdowns: " << statShutdowns_
        << " Data: " << statData_
        << " HeartbeatWithoutPublish: " << statHeartbeatWithoutPublish_
        << " ShutdownPublishedGap: " << statShutdownPublishedGap_
        << " ArrivedInOrder: " << statArrivedInOrder_
        << " DuplicatesPrevious: " << statDuplicatesPrevious_
        << " Stashed: " << statStashed_
        << " DuplicatesStash: " << statDuplicatesStash_
        << " Future: " << statFuture_;
}
#endif //DISABLED
