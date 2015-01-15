// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/Consumer.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<typename CargoMessage, typename GapMessage>
        class Arbitrator : public std::enable_shared_from_this<Arbitrator<CargoMessage, GapMessage> >
        {
        public:
            Arbitrator(ConnectionPtr & inConnection, ConnectionPtr & outConnection, size_t lookAhead, bool quitOnEmptyMessage = true);
            ~Arbitrator();

            void start();
            void stop();
            void pause();
            void resume();

            void run();
        private:
            void handleEmptyMessage(Message & message);
            void handleMessageType(Message::Meta::MessageType type, Message & message);
            void handleHeartbeat(Message & message);
            void handleDataMessage(Message & message);

        private:
            ConnectionPtr inConnection_;
            ConnectionPtr outConnection_;
            Consumer consumer_;
            Producer producer_;
            Message message_;
            size_t lookAhead_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;

            uint32_t expectedSequenceNumber_;
            std::vector<Message> messages_;
            
            std::shared_ptr<Arbitrator> me_;
            std::thread thread_;
        };

        template<typename CargoMessage, typename GapMessage>
        Arbitrator<CargoMessage, GapMessage>::Arbitrator(ConnectionPtr & inConnection, ConnectionPtr & outConnection, size_t lookAhead, bool quitOnEmptyMessage)
            : inConnection_(inConnection)
            , outConnection_(outConnection)
            , consumer_(inConnection_)
            , producer_(outConnection_)
            , message_(inConnection)
            , lookAhead_(lookAhead)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , paused_(false)
            , stopping_(false)
            , expectedSequenceNumber_(0)
        {
            for(auto nMessage = 0; nMessage < lookAhead; ++nMessage)
            {
                messages_.emplace_back(outConnection_);
            }
        }

        template<typename CargoMessage, typename GapMessage>
        Arbitrator<CargoMessage, GapMessage>::~Arbitrator()
        {
            stop();
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::start()
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                &Arbitrator<CargoMessage, GapMessage>::run,
                this));
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::stop()
        {
            stopping_ = true;
            if(me_)
            { 
                thread_.join();
                me_.reset();
            }
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::pause()
        {
            paused_ = true;
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::resume()
        {
            paused_ = false;
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::run()
        {
            DebugMessage("Arbitrator start.\n");
            while(!stopping_)
            {
                stopping_ = !consumer_.getNext(message_);
                if(!stopping_ && message_.isEmpty())
                {
                    handleEmptyMessage(message_);
                }
                if(!stopping_)
                {
                    handleMessageType(message_.meta().type_, message_);
                }
            }
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::handleEmptyMessage(Message & message)
        {
            DebugMessage("Arbitrator received empty message" << std::endl);
            producer_.publish(message);
            stopping_ = quitOnEmptyMessage_;
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::handleMessageType(Message::Meta::MessageType type, Message & message)
        {
            if(type == Message::Meta::Heartbeat)
            {
                handleHeartbeat(message);
            }
            else
            {
                handleDataMessage(message);
            }
        }

        template<typename CargoMessage, typename GapMessage>
        void Arbitrator<CargoMessage, GapMessage>::handleHeartbeat(Message & message)
        {
            // todo:
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
                if(messages_[index].isEmpty())
                {
                    DebugMessage("Stash" << std::endl);
                    message.moveTo(messages_[index]);
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
                    if(messages_[index].isEmpty())
                    {
                        DebugMessage("Gap " << expectedSequenceNumber_ << std::endl);
                        messages_[index].appendEmplace<GapMessage>(expectedSequenceNumber_);
                    }
                    else
                    {
                        DebugMessage("publish Future " << expectedSequenceNumber_ << std::endl);
                    }
                    producer_.publish(messages_[index]);
                    ++expectedSequenceNumber_;
                }
                DebugMessage("Stash future" << std::endl);
                auto index = sequence % lookAhead_;
                message.moveTo(messages_[index]);
            }
            // flush any additional messages
            auto index = expectedSequenceNumber_ % lookAhead_;
            while(!messages_[index].isEmpty())
            {
                DebugMessage("Also publish " << expectedSequenceNumber_ << std::endl);
                producer_.publish(messages_[index]);
                ++expectedSequenceNumber_;
                index = expectedSequenceNumber_ % lookAhead_;
            }

        }

   }
}