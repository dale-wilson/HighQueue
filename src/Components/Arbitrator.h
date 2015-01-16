// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ComponentCommon/MessageProcessor.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<typename CargoMessage, typename GapMessage>
        class Arbitrator : public MessageProcessor
        {
        public:
            Arbitrator(ConnectionPtr & inConnection, ConnectionPtr & outConnection, size_t lookAhead, bool quitOnEmptyMessage = true);

        private:
            virtual bool handleEmptyMessage(Message & message);
            virtual bool handleMessageType(Message::Meta::MessageType type, Message & message);
            virtual bool handleHeartbeat(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;

            uint32_t expectedSequenceNumber_;
            std::vector<Message> pendingMessages_;
        };

        template<typename CargoMessage, typename GapMessage>
        Arbitrator<CargoMessage, GapMessage>::Arbitrator(ConnectionPtr & inConnection, ConnectionPtr & outConnection, size_t lookAhead, bool quitOnEmptyMessage)
            : MessageProcessor(inConnection, outConnection)
            , lookAhead_(lookAhead)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , paused_(false)
            , stopping_(false)
            , expectedSequenceNumber_(0)
        {
            for(auto nMessage = 0; nMessage < lookAhead; ++nMessage)
            {
                pendingMessages_.emplace_back(outConnection_);
            }
        }

        template<typename CargoMessage, typename GapMessage>
        bool Arbitrator<CargoMessage, GapMessage>::handleEmptyMessage(Message & message)
        {
            DebugMessage("Arbitrator received empty message" << std::endl);
            producer_.publish(message);
            return !quitOnEmptyMessage_;
        }

        template<typename CargoMessage, typename GapMessage>
        bool Arbitrator<CargoMessage, GapMessage>::handleMessageType(Message::Meta::MessageType type, Message & message)
        {
            // todo validate message type?
            handleDataMessage(message);
            return true;
        }

        template<typename CargoMessage, typename GapMessage>
        bool Arbitrator<CargoMessage, GapMessage>::handleHeartbeat(Message & message)
        {
            std::cerr << "Arbitrator received heartbeat\n";
            // todo:
            return true;
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::handleDataMessage(Message & message)
        {
            auto testMessage = message.get<CargoMessage>();
            auto sequence = testMessage->getSequence();
            if(expectedSequenceNumber_ == 0)
            {
                expectedSequenceNumber_ = sequence;
            }
            DebugMessage("Arbitrator sequence :" << sequence << " expected " << expectedSequenceNumber_ << std::endl);
            if(sequence == expectedSequenceNumber_)
            {
                DebugMessage("Publish " << std::endl);
                producer_.publish(message);
                ++expectedSequenceNumber_;
            }
            else if(sequence < expectedSequenceNumber_)
            {
                DebugMessage("Ancient" << std::endl);
                // ignore this one.  We've already seen it.
            }
            else if(sequence - expectedSequenceNumber_ < lookAhead_)
            {
                auto index = sequence % lookAhead_;
                if(pendingMessages_[index].isEmpty())
                {
                    DebugMessage("Stash" << std::endl);
                    message.moveTo(pendingMessages_[index]);
                }
                else
                {
                    DebugMessage("Duplicate" << std::endl);
                    // ignore this one we are already holding a copy.
                }
            }
            else // Sequence number is beyond the look-ahead window size
            {
                while(sequence - expectedSequenceNumber_ < lookAhead_)
                {
                    auto index = expectedSequenceNumber_ % lookAhead_;
                    if(pendingMessages_[index].isEmpty())
                    {
                        DebugMessage("Gap " << expectedSequenceNumber_ << std::endl);
                        pendingMessages_[index].emplace<GapMessage>(expectedSequenceNumber_);
                    }
                    else
                    {
                        DebugMessage("publish Future " << expectedSequenceNumber_ << std::endl);
                    }
                    producer_.publish(pendingMessages_[index]);
                    ++expectedSequenceNumber_;
                }
                DebugMessage("Stash future" << std::endl);
                auto index = sequence % lookAhead_;
                message.moveTo(pendingMessages_[index]);
            }
            // flush any additional messages
            auto index = expectedSequenceNumber_ % lookAhead_;
            while(!pendingMessages_[index].isEmpty())
            {
                DebugMessage("Also publish " << expectedSequenceNumber_ << std::endl);
                producer_.publish(pendingMessages_[index]);
                ++expectedSequenceNumber_;
                index = expectedSequenceNumber_ % lookAhead_;
            }
        }
   }
}