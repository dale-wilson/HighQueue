// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StageCommon/StageToMessage.h>

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

        template<typename CargoMessage>
        class Arbitrator : public StageToMessage
        {
        public:
            Arbitrator(size_t lookAhead, size_t expectedShutdowns = 2);


            virtual void attachConnection(const ConnectionPtr & connection);
            virtual void attachMemoryPool(const MemoryPoolPtr & memoryPool);

            virtual void validate();
            virtual void handle(Message & message);

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
        };

        template<typename CargoMessage>
        Arbitrator<CargoMessage>::Arbitrator(size_t lookAhead, size_t expectedShutdowns)
            : lookAhead_(lookAhead)
            , expectedShutdowns_(expectedShutdowns)
            , actualShutdowns_(0)
            , expectedSequenceNumber_(0)
            , lastHeartbeatSequenceNumber_(0)
        {
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::attachConnection(const ConnectionPtr & connection)
        {      
            while(pendingMessages_.size() < lookAhead_)
            {
                pendingMessages_.emplace_back(connection);
            }
            StageToMessage::attachConnection(connection);
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::attachMemoryPool(const MemoryPoolPtr & memoryPool)
        {
            while(pendingMessages_.size() < lookAhead_)
            {
                pendingMessages_.emplace_back(memoryPool);
            }
            StageToMessage::attachMemoryPool(memoryPool);
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::validate()
        {
            if(pendingMessages_.size() < lookAhead_)
            {
                throw std::runtime_error("Arbitrator working messages not initialized. Missing call to attachConnection or attachMemoryPool?");
            }
            StageToMessage::validate();
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::handle(Message & message)
        {
            auto type = message.meta().type_;
            if(type == Message::Meta::MessageType::Heartbeat)
            {
                handleHeartbeat(message);
            }
            else if(type == Message::Meta::MessageType::Shutdown)
            {
                handleShutdown(message);
            }
            else
            {
                handleDataMessage(message);
            }
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::handleHeartbeat(Message & message)
        {
            // todo: we might want to skip some heartbeats depending on frequency
            if(expectedSequenceNumber_ == lastHeartbeatSequenceNumber_)
            {
                findAndPublishGap();
                publishPendingMessages();
            }
            lastHeartbeatSequenceNumber_ = expectedSequenceNumber_;
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::handleShutdown(Message & message)
        {
            ++actualShutdowns_;
            LogTrace("Arbitration received shutdown " << actualShutdowns_ << " of " << expectedShutdowns_);
            if(actualShutdowns_ == expectedShutdowns_)
            {
                while(findAndPublishGap())
                {
                    publishPendingMessages();
                }
                // forward the shutdown message
                send(message);
                stop();
            }
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::handleDataMessage(Message & message)
        {
            auto testMessage = message.get<CargoMessage>();
            auto sequence = testMessage->getSequence();
            if(expectedSequenceNumber_ == 0)
            {
                expectedSequenceNumber_ = sequence;
            }
            LogVerbose("Arbitrator sequence :" << sequence << " expected " << expectedSequenceNumber_);
            if(sequence == expectedSequenceNumber_)
            {
                LogDebug("Arbitrator Publish " << sequence);
                send(message);
                ++expectedSequenceNumber_;
                publishPendingMessages();
            }
            else if(sequence < expectedSequenceNumber_)
            {
                LogDebug("Arbitrator Previous Duplicate " << sequence);
                // ignore this one.  We've already seen it.
            }
            else if(sequence - expectedSequenceNumber_ < lookAhead_)
            {
                auto index = sequence % lookAhead_;
                if(pendingMessages_[index].isEmpty())
                {
                    LogDebug("Arbitrator Stash" << sequence);
                    message.moveTo(pendingMessages_[index]);
                }
                else
                {
                    LogDebug("Arbitrator Duplicates Stash " << sequence);
                    // ignore this one we are already holding a copy.
                }
            }
            else // Sequence number is beyond the look-ahead window size
            {
                while(sequence - expectedSequenceNumber_ < lookAhead_)
                {
                    if(findAndPublishGap())
                    {
                        publishPendingMessages();
                    }
                    else
                    {
                        publishGapMessage(expectedSequenceNumber_, sequence);
                        send(message);
                    }
                }
                LogDebug("Arbitrator Stash future " << sequence);
                auto index = sequence % lookAhead_;
                message.moveTo(pendingMessages_[index]);
            }
        }

        template<typename CargoMessage>
        bool Arbitrator<CargoMessage>::findAndPublishGap()
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

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::publishGapMessage(uint32_t gapStart, uint32_t gapEnd)
        {
            outMessage_->meta().type_ = Message::Meta::Gap;
            outMessage_->emplace<GapMessage>(gapStart, gapEnd - 1);
            expectedSequenceNumber_ = gapEnd;
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::publishPendingMessages()
        {
            auto index = expectedSequenceNumber_ % lookAhead_;
            while(!pendingMessages_[index].isEmpty())
            {
                LogDebug("Arbitrator Publish from stash " << expectedSequenceNumber_ );
                send(pendingMessages_[index]);
                ++expectedSequenceNumber_;
                index = expectedSequenceNumber_ % lookAhead_;
            }
        }
   }
}