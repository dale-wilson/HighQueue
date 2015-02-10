// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.h>
#include <Mocks/MockMessage.h>
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

        private:
            uint32_t messageCount_;

            uint32_t heartbeats_;
            uint32_t shutdowns_;
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
            , messagesHandled_(0)
            , nextSequence_(0)
            , sequenceError_(0)
            , unexpectedMessageError_(0)
        {
            setName("MockMessageConsumer"); // default name
        }

        template<typename MockMessageType>
        bool MockMessageConsumer<MockMessageType>::configureParameter(const std::string & key, const ConfigurationNode & configuration)
        {
            if(key == keyMessageCount)
            {
                uint64_t messageCount;
                if(!configuration.getValue(messageCount))
                {
                    LogFatal("TestMessagConsumer can't interpret value for " << keyMessageCount);
                }
                messageCount_ = uint32_t(messageCount);
                return true;
            }

            return false;
        }

        template<typename MockMessageType>
        void MockMessageConsumer<MockMessageType>::handle(Message & message)
        {
            auto type = message.getType();
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
                    LogDebug("MockMessageConsumer: " << testMessage->getSequence());
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
            }
        }

        template<typename MockMessageType>
        void MockMessageConsumer<MockMessageType>::finish()
        {
            LogStatistics("MockMessageConsumer::heartbeats:" << heartbeats_);
            LogStatistics("MockMessageConsumer::shutdowns: "  << shutdowns_);
            LogStatistics("MockMessageConsumer::messagesHandled:" << messagesHandled_);
            LogStatistics("MockMessageConsumer::sequenceError:" << sequenceError_);
            LogStatistics("MockMessageConsumer::unexpectedMessageError:" << unexpectedMessageError_);
            Step::finish();
        }

   }
}