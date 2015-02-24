// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.h>

#include "OrderedMerge.h"
#include <Steps/StepFactory.h>
#include <Steps/SharedResources.h>
#include <Steps/Configuration.h>
#include <HighQueue/MemoryPool.h>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<OrderedMerge> registerStepSmall("ordered_merge", "Merge streams of messages, sorting, eliminating duplicates, identifying gaps.");

    const std::string keyLookAhead = "look_ahead";
}

std::ostream & OrderedMerge::usage(std::ostream & out) const
{
    out << "    " << keyLookAhead << ": The maximum number of messages to keep before declaring a gap (missing message)." << std::endl;
    return StepToMessage::usage(out);
}


OrderedMerge::OrderedMerge()
    : lookAhead_(0)
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
}

bool OrderedMerge::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyLookAhead)
    {
        uint64_t lookAhead;
        if(configuration.getValue(lookAhead))
        {
            lookAhead_ = size_t(lookAhead);
            return true;
        }
    }
    else
    {
        return StepToMessage::configureParameter(key, configuration);
    }
    return false;
}

void OrderedMerge::configureResources(SharedResources & resources)
{
    resources.requestMessageSize(sizeof(GapMessage));
    resources.requestMessages(lookAhead_);
    StepToMessage::configureResources(resources);
}

void OrderedMerge::attachResources(SharedResources & resources)
{
    auto & memoryPool = resources.getMemoryPool();
    if(!memoryPool)
    {
        throw std::runtime_error("OrderedMerge: no memory pool available.");
    }
    while(pendingMessages_.size() < lookAhead_)
    {
        pendingMessages_.emplace_back(new Message(memoryPool));
    }

    StepToMessage::attachResources(resources);
}

void OrderedMerge::validate()
{
    if(pendingMessages_.size() < lookAhead_)
    {
        throw std::runtime_error("OrderedMerge: Pending messages not allocated.");
    }
    StepToMessage::validate();
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
        if(findAndPublishGap())
        {
            ++statHeartbeatWithoutPublish_;
            publishPendingMessages();
        }
    }
    lastHeartbeatSequenceNumber_ = expectedSequenceNumber_;
    send(message);
}

void OrderedMerge::handleShutdown(Message & message)
{
    LogTrace("OrderedMerge received shutdown " << statShutdowns_ );
    while(findAndPublishGap())
    {
        ++statShutdownPublishedGap_;
        publishPendingMessages();
    }
    // forward the shutdown message
    send(message);
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
        if(pendingMessages_[index]->isEmpty())
        {
            ++statStashed_;
            LogDebug("OrderedMerge Stash" << sequence);
            message.moveTo(*pendingMessages_[index]);
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
        message.moveTo(*pendingMessages_[index]);
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
        if(!pendingMessages_[index]->isEmpty())
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
    while(!pendingMessages_[index]->isEmpty())
    {
        LogDebug("OrderedMerge Publish from stash " << expectedSequenceNumber_ );
        send(*pendingMessages_[index]);
        ++expectedSequenceNumber_;
        index = expectedSequenceNumber_ % lookAhead_;
    }
}

void OrderedMerge::finish()
{
    LogStatistics("OrderedMerge received: " << statReceived_);
    LogStatistics("OrderedMerge heartbeat: " << statHeartbeats_);
    LogStatistics("OrderedMerge shutdown: " << statShutdowns_);
    LogStatistics("OrderedMerge data: " << statData_);
    LogStatistics("OrderedMerge heartbeat gap: " << statHeartbeatWithoutPublish_);
    LogStatistics("OrderedMerge shutdown gap: " << statShutdownPublishedGap_);
    LogStatistics("OrderedMerge in_order: " << statArrivedInOrder_);
    LogStatistics("OrderedMerge duplicate_published: " << statDuplicatesPrevious_);
    LogStatistics("OrderedMerge stashed: " << statStashed_);
    LogStatistics("OrderedMerge duplicate_stashed: " << statDuplicatesStash_);
    LogStatistics("OrderedMerge future gap: " << statFuture_);
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

