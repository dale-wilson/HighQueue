// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Mocks/MockMessage.h>
#include <StepLibrary/GapMesssage.h>
#include <Steps/Step.h>
#include <Steps/Configuration.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        template<typename MockMessageType>
        class MockMessageConsumer : public Step
        {
        public:
            typedef MockMessageType ActualMessage;
            static const std::string keyMessageCount;

            explicit MockMessageConsumer(uint32_t messageCount_ = 0);
         
            uint32_t errors()const
            {
                return sequenceError_ + unexpectedMessageError_;
            }
            uint32_t messagesHandled()const
            {
                return messagesHandled_;
            }
            uint32_t heartbeatsReceived()const
            {
                return heartbeats_;
            }

            ////////////////////////////
            // Implement Step
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void handle(Message & message);
            virtual void finish();
            virtual void logStats();
        
        private:
            uint32_t messageCount_;

            uint32_t heartbeats_;
            uint32_t shutdowns_;
            uint32_t gaps_;
            uint32_t messagesHandled_;
            uint32_t nextSequence_;
            uint32_t sequenceError_;
            uint32_t unexpectedMessageError_;
        };

        template<typename MockMessageType>
        const std::string MockMessageConsumer<MockMessageType>::keyMessageCount = "message_count";

        template<typename MockMessageType>
        MockMessageConsumer<MockMessageType>::MockMessageConsumer(uint32_t messageCount_)
            : messageCount_(messageCount_)
            , heartbeats_(0)
            , shutdowns_(0)
            , gaps_(0)
            , messagesHandled_(0)
            , nextSequence_(0)
            , sequenceError_(0)
            , unexpectedMessageError_(0)
        {
        }

        template<typename MockMessageType>
        bool MockMessageConsumer<MockMessageType>::configureParameter(const std::string & key, const ConfigurationNode & configuration)
        {
            if(key == keyMessageCount)
            {
                uint64_t messageCount;
                if(!configuration.getValue(messageCount))
                {
                    LogFatal("MockMessageConsumer can't interpret value for " << keyMessageCount);
                }
                messageCount_ = uint32_t(messageCount);
                return true;
            }
            return Step::configureParameter(key, configuration);
        }

        template<typename MockMessageType>
        void MockMessageConsumer<MockMessageType>::handle(Message & message)
        {
            auto type = message.getType();
            if(type == Message::MulticastPacket)
            {
                type = Message::MockMessage;
                message.setType(type);
            }
            switch(type)
            {
                default:
                {
                    LogError("MockMessageConsumer::Expecting test message, not " << Message::toText(type));
                    ++unexpectedMessageError_;  
                    return;
                }
                case Message::Shutdown:
                {
                    if (messageCount_ == 0)
                    {
                        LogTrace("MockMessageConsumer: received Shutdown");
                        ++shutdowns_;
                        stop();
                    }
                    return;
                }
                case Message::MockMessage:
                {
                    auto testMessage = message.get<ActualMessage>();
                    if(nextSequence_ != testMessage->getSequence())
                    {
                        LogWarningLimited(10, "Expecting " << nextSequence_ << " received " << testMessage->getSequence());
                        ++sequenceError_;
                        nextSequence_ = testMessage->getSequence();
                    }
                    ++nextSequence_;
                    message.destroy<ActualMessage>();
                    ++messagesHandled_;
                    if(messageCount_ != 0 && messagesHandled_ >= messageCount_)
                    {
                        stop();
                    }
                    return;
                }
                case Message::Heartbeat:
                {
                    ++heartbeats_;
                }
                case Message::Gap:
                {
                    auto gapMessage = message.get<GapMessage>();
                    LogTrace("MockMessageConsumer " << name_ << " received gap [" << gapMessage->startGap() << ", " << gapMessage->endGap() << ")");
                    nextSequence_ = gapMessage->endGap();
                    ++gaps_;
                }
            }
        }

        template<typename MockMessageType>
        void MockMessageConsumer<MockMessageType>::finish()
        {
            logStats();
        }

        template<typename MockMessageType>
        void MockMessageConsumer<MockMessageType>::logStats()
        {
            LogStatistics("MockMessageConsumer " << name_ << " heartbeats:" << heartbeats_);
            LogStatistics("MockMessageConsumer " << name_ << " shutdowns: " << shutdowns_);
            LogStatistics("MockMessageConsumer " << name_ << " messagesHandled:" << messagesHandled_);
            LogStatistics("MockMessageConsumer " << name_ << " sequenceError:" << sequenceError_);
            LogStatistics("MockMessageConsumer " << name_ << " unexpectedMessageError:" << unexpectedMessageError_);
            Step::finish();
        }

   }
}