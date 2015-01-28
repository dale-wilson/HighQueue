// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "OrderedMergeFwd.h"

#include <StagesSupport/StageToMessage.h>

namespace HighQueue
{
    namespace Stages
    {
        class GapMessage
        {
        public:
            GapMessage(uint32_t startGap, uint32_t gapEnd = 0)
                : startGap_(startGap)
                , endGap_(gapEnd)
            {
            }
            uint32_t & startGap()
            {
                return startGap_;
            }
            uint32_t & gapEnd()
            {
                return endGap_;
            }
        private:
            uint32_t startGap_;
            uint32_t endGap_;
        };

        class OrderedMerge : public StageToMessage
        {
        public:
            OrderedMerge(size_t lookAhead = 0, size_t expectedShutdowns = 2);

            virtual void attachConnection(const ConnectionPtr & connection);
            virtual void attachMemoryPool(const MemoryPoolPtr & memoryPool);

            virtual void validate();
            virtual void handle(Message & message);

            std::ostream & writeStats(std::ostream & out);

        private:
            bool findAndPublishGap();
            void publishGapMessage(uint32_t gapStart, uint32_t gapEnd);
            void publishPendingMessages();

            void handleHeartbeat(Message & message);
            void handleShutdown(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            size_t expectedShutdowns_;
            size_t actualShutdowns_;
            uint32_t expectedSequenceNumber_;
            std::vector<Message> pendingMessages_;
            uint32_t lastHeartbeatSequenceNumber_;

            size_t statReceived_;
            size_t statHeartbeats_;
            size_t statShutdowns_;
            size_t statData_;
            size_t statHeartbeatWithoutPublish_;
            size_t statShutdownPublishedGap_;
            size_t statArrivedInOrder_;
            size_t statDuplicatesPrevious_;
            size_t statStashed_;
            size_t statDuplicatesStash_;
            size_t statFuture_;
        };

        inline
        OrderedMerge::OrderedMerge(size_t lookAhead, size_t expectedShutdowns)
            : lookAhead_(lookAhead)
            , expectedShutdowns_(expectedShutdowns)
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

        inline
        void OrderedMerge::attachConnection(const ConnectionPtr & connection)
        {      
            while(pendingMessages_.size() < lookAhead_)
            {
                pendingMessages_.emplace_back(connection);
            }
            StageToMessage::attachConnection(connection);
        }

        inline
        void OrderedMerge::attachMemoryPool(const MemoryPoolPtr & memoryPool)
        {
            while(pendingMessages_.size() < lookAhead_)
            {
                pendingMessages_.emplace_back(memoryPool);
            }
            StageToMessage::attachMemoryPool(memoryPool);
        }

        inline
        void OrderedMerge::validate()
        {
            if(pendingMessages_.size() < lookAhead_)
            {
                throw std::runtime_error("OrderedMerge working messages not initialized. Missing call to attachConnection or attachMemoryPool?");
            }
            StageToMessage::validate();
        }

        inline
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

        inline
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

        inline
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

        inline
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

        inline
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

        inline
        void OrderedMerge::publishGapMessage(uint32_t gapStart, uint32_t gapEnd)
        {
            outMessage_->setType(Message::Gap);
            outMessage_->emplace<GapMessage>(gapStart, gapEnd - 1);
            expectedSequenceNumber_ = gapEnd;
        }

        inline
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

        inline
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

   }
}