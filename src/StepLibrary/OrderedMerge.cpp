// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <Steps/StepPch.hpp>

#include "OrderedMerge.hpp"
#include <Steps/StepFactory.hpp>
#include <Steps/SharedResources.hpp>
#include <Steps/Configuration.hpp>
#include <HighQueue/MemoryPool.hpp>

using namespace HighQueue;
using namespace Steps;

namespace
{
    StepFactory::Registrar<OrderedMerge> registerStepSmall("ordered_merge", "Merge streams of messages, sorting, eliminating duplicates, identifying gaps.");

    const std::string keyLookAhead = "look_ahead";
    const std::string keyDelayHeartbeats = "max_delay_heartbeats";
}

std::ostream & OrderedMerge::usage(std::ostream & out) const
{
    out << "    " << keyLookAhead << ": The maximum number of messages to keep before declaring a gap (missing message(s))." << std::endl;
    out << "    " << keyDelayHeartbeats << ": The maximum number of heartbeats to delay before declaring a gap." << std::endl;
    return StepToMessage::usage(out);
}


OrderedMerge::OrderedMerge()
    : lookAhead_(0)
    , maxDelayHeartbeats_(1)
    , heartbeatDelays_(0)            
    , expectedSequenceNumber_(0)
    , highestStashed_(0)
    , lastHeartbeatSequenceNumber_(0)
    , lastHeartbeatHighestStashed_(0)
    , statReceived_(0)
    , statHeartbeats_(0)
    , statShutdowns_(0)
    , statData_(0)
    , statHeartbeatWithoutPublish_(0)
    , statShutdownPublishedGap_(0)
    , statArrivedInOrder_(0)
    , statPrevious_(0)
    , statStashed_(0)
    , statDuplicatesStash_(0)
    , statFuture_(0)
{
}

bool OrderedMerge::configureParameter(const std::string & key, const ConfigurationNode & configuration)
{
    if(key == keyLookAhead)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            lookAhead_ = size_t(value);
            return true;
        }
    }
    else if(key == keyDelayHeartbeats)
    {
        uint64_t value;
        if(configuration.getValue(value))
        {
            maxDelayHeartbeats_ = size_t(value);
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
    if(expectedSequenceNumber_ == lastHeartbeatSequenceNumber_)
    {
        ++heartbeatDelays_;
        if(heartbeatDelays_ >= maxDelayHeartbeats_)
        {
            while(expectedSequenceNumber_ < lastHeartbeatHighestStashed_)
            {
                if(findAndPublishGap())
                {
                    ++statHeartbeatWithoutPublish_;
                    publishPendingMessages();
                }
            }
            lastHeartbeatSequenceNumber_ = expectedSequenceNumber_;
            lastHeartbeatHighestStashed_ = highestStashed_;
            heartbeatDelays_ = 0;
        }
    }
    else
    {
        lastHeartbeatSequenceNumber_ = expectedSequenceNumber_;
        lastHeartbeatHighestStashed_ = highestStashed_;
        heartbeatDelays_ = 0;
    }
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
        highestStashed_ = sequence;
    }
    if(sequence == expectedSequenceNumber_)
    {
        ++statArrivedInOrder_;
        LogDebug("OrderedMerge Publish immediate" << sequence);
        send(message);
        ++expectedSequenceNumber_;
        publishPendingMessages();
    }
    else if(sequence < expectedSequenceNumber_)
    {
        ++statPrevious_;
        LogDebug("OrderedMerge ignore ancient " << sequence);
        // ignore this one.  We've already seen it.
    }
    else if(sequence - expectedSequenceNumber_ < lookAhead_)
    {
        auto index = sequence % lookAhead_;
        if(pendingMessages_[index]->isEmpty())
        {
            ++statStashed_;
            LogDebug("OrderedMerge Stash" << sequence << " in " << index);
            message.moveTo(*pendingMessages_[index]);
            if(sequence > highestStashed_)
            {
                highestStashed_ = sequence;
            }
        }
        else
        {
            LogDebug("OrderedMerge Duplicates Stash " << sequence << " :: [" << index << "] " << pendingMessages_[index]->getSequence());
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
        highestStashed_ = sequence;
    }
}

bool OrderedMerge::findAndPublishGap()
{
    auto gapStart = expectedSequenceNumber_;
    auto index = gapStart % lookAhead_;
    if(pendingMessages_[index]->isEmpty())
    {
        auto gapEnd = gapStart + 1; // the first message that's NOT in the gap
        auto endLookahead = gapStart + lookAhead_;
        LogDebug("Find and publish gap looking for unpublished messages in [" << gapStart << ", " << endLookahead << ')');
        while(gapEnd < endLookahead)
        {
            index = gapEnd % lookAhead_;
            if(!pendingMessages_[index]->isEmpty())
            {
                LogDebug("Find and publish gap unpublished message: Publishing gap  [" << gapStart << ", " << gapEnd << ')');
                publishGapMessage(gapStart, gapEnd);
                return true;
            }
            ++gapEnd;
        }
        LogDebug("Find and publish gap found no unpublished messages, hence no gap.");
        return false;
    }
    LogDebug("Find and publish gap: First message was unpublished at " << gapStart);
    return true;
}

void OrderedMerge::publishGapMessage(uint32_t gapStart, uint32_t gapEnd)
{
    LogDebug("Publish Gap message [" << gapStart << ", " << gapEnd << ']');
    outMessage_->setType(Message::Gap);
    outMessage_->setSequence(gapEnd);
    outMessage_->emplace<GapMessage>(gapStart, gapEnd - 1);
    expectedSequenceNumber_ = gapEnd;
}

void OrderedMerge::publishPendingMessages()
{
    auto index = expectedSequenceNumber_ % lookAhead_;
    while(!pendingMessages_[index]->isEmpty())
    {
        LogDebug("OrderedMerge Publish from stash " << expectedSequenceNumber_ << '[' << index << "] = " << pendingMessages_[index]->getSequence() );
        send(*pendingMessages_[index]);
        ++expectedSequenceNumber_;
        index = expectedSequenceNumber_ % lookAhead_;
    }
}

void OrderedMerge::finish()
{
    publishStats();
}

void OrderedMerge::publishStats()
{
    LogStatistics("OrderedMerge "<< name_ <<" received: " << statReceived_);
    LogStatistics("OrderedMerge "<< name_ <<" heartbeat: " << statHeartbeats_);
    LogStatistics("OrderedMerge "<< name_ <<" shutdown: " << statShutdowns_);
    LogStatistics("OrderedMerge "<< name_ <<" data: " << statData_);
    LogStatistics("OrderedMerge "<< name_ <<" heartbeat gap: " << statHeartbeatWithoutPublish_);
    LogStatistics("OrderedMerge "<< name_ <<" shutdown gap: " << statShutdownPublishedGap_);
    LogStatistics("OrderedMerge "<< name_ <<" in_order: " << statArrivedInOrder_);
    LogStatistics("OrderedMerge "<< name_ <<" duplicate_published: " << statPrevious_);
    LogStatistics("OrderedMerge "<< name_ <<" stashed: " << statStashed_);
    LogStatistics("OrderedMerge "<< name_ <<" duplicate_stashed: " << statDuplicatesStash_);
    LogStatistics("OrderedMerge "<< name_ <<" future gap: " << statFuture_);
}

