// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ComponentCommon/MessageProcessor.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class GapMessage
        {
        public:
            GapMessage(uint32_t startGap, uint32_t gapEnd = 0)
                : startGap_(startGap)
                , endGap_(gapEnd)
            {
            }
            uint32_t & startGap();
            uint32_t & gapEnd();
        private:
            uint32_t startGap_;
            uint32_t endGap_;
        };

        template<typename CargoMessage>
        class Arbitrator : public MessageProcessor
        {
        public:
            Arbitrator(ConnectionPtr & inConnection, ConnectionPtr & outConnection, size_t lookAhead, bool quitOnEmptyMessage = true);

        private:
            virtual bool handleEmptyMessage(Message & message);
            virtual bool handleMessageType(Message::Meta::MessageType type, Message & message);
            virtual bool handleHeartbeat(Message & message);
            void handleDataMessage(Message & message);

            bool findAndPublishGap();
            void publishGapMessage(uint32_t gapStart, uint32_t gapEnd);
            void publishPendingMessages();

        private:
            size_t lookAhead_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;

            uint32_t expectedSequenceNumber_;
            std::vector<Message> pendingMessages_;
            uint32_t lastHeartbeatSequenceNumber_;
        };

        template<typename CargoMessage>
        Arbitrator<CargoMessage>::Arbitrator(ConnectionPtr & inConnection, ConnectionPtr & outConnection, size_t lookAhead, bool quitOnEmptyMessage)
            : MessageProcessor(inConnection, outConnection)
            , lookAhead_(lookAhead)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , paused_(false)
            , stopping_(false)
            , expectedSequenceNumber_(0)
            , lastHeartbeatSequenceNumber_(0)
        {
            for(auto nMessage = 0; nMessage < lookAhead; ++nMessage)
            {
                pendingMessages_.emplace_back(outConnection_);
            }
        }

        template<typename CargoMessage>
        bool Arbitrator<CargoMessage>::handleEmptyMessage(Message & message)
        {
            publish(message);
            return !quitOnEmptyMessage_;
        }

        template<typename CargoMessage>
        bool Arbitrator<CargoMessage>::handleMessageType(Message::Meta::MessageType type, Message & message)
        {
            // todo validate message type?
            handleDataMessage(message);
            return true;
        }

        template<typename CargoMessage>
        bool Arbitrator<CargoMessage>::handleHeartbeat(Message & message)
        {
            // todo: we might want to skip some heartbeats depending on frequency
            if(expectedSequenceNumber_ == lastHeartbeatSequenceNumber_)
            {
                findAndPublishGap();
                void publishPendingMessages();
            }
            lastHeartbeatSequenceNumber_ = expectedSequenceNumber_;
            // todo:
            return true;
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
                publish(message);
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
                        publish(message);
                    }
                }
                LogDebug("Arbitrator Stash future " << sequence);
                auto index = sequence % lookAhead_;
                message.moveTo(pendingMessages_[index]);
            }
            // flush any additional messages
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
            }
            return false;
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::publishGapMessage(uint32_t gapStart, uint32_t gapEnd)
        {
            outMessage_.meta().type_ = Message::Meta::Gap;
            outMessage_.emplace<GapMessage>(gapStart, gapEnd - 1);
            expectedSequenceNumber_ = gapEnd;
        }

        template<typename CargoMessage>
        void Arbitrator<CargoMessage>::publishPendingMessages()
        {
            auto index = expectedSequenceNumber_ % lookAhead_;
            while(!pendingMessages_[index].isEmpty())
            {
                LogDebug("Arbitrator Publish from stash " << expectedSequenceNumber_ );
                publish(pendingMessages_[index]);
                ++expectedSequenceNumber_;
                index = expectedSequenceNumber_ % lookAhead_;
            }
        }

   }
}