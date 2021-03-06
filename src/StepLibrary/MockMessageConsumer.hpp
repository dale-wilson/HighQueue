// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Mocks/MockMessage.hpp>
#include <StepLibrary/GapMesssage.hpp>
#include <Steps/Step.hpp>
#include <Steps/Configuration.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class BaseMessageConsumer : public Step
        {
        public:
            BaseMessageConsumer();
            virtual ~BaseMessageConsumer();

            ////////////////////////////
            // Implement Step
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration) override;
            virtual void logStats() override;

        protected:
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
        class MockMessageConsumer : public BaseMessageConsumer
        {
        public:
            typedef MockMessageType ActualMessage;
            static const std::string keyMessageCount;

            explicit MockMessageConsumer(uint32_t messageCount_ = 0);         

            ////////////////////////////
            // Implement Step
            virtual void handle(Message & message) override;
        
        };

        template<typename MockMessageType>
        MockMessageConsumer<MockMessageType>::MockMessageConsumer(uint32_t messageCount)
        {
            messageCount_ = messageCount;
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
                case Message::MessageType::Shutdown:
                {
                    if (messageCount_ == 0)
                    {
                        LogTrace("MockMessageConsumer: received Shutdown");
                        ++shutdowns_;
                        stop();
                    }
                    return;
                }
                case Message::MessageType::MockMessage:
                {
                    auto testMessage = message.get<ActualMessage>();
                    if(nextSequence_ != testMessage->getSequence())
                    {
                        LogWarningLimited(10, "MockMessageConsumer Expecting " << nextSequence_ << " received " << testMessage->getSequence());
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
                case Message::MessageType::Heartbeat:
                {
                    ++heartbeats_;
                    return;
                }
                case Message::MessageType::Gap:
                {
                    auto gapMessage = message.get<GapMessage>();
                    LogTrace("MockMessageConsumer " << name_ << " received gap [" << gapMessage->startGap() << ", " << gapMessage->endGap() << ")");
                    nextSequence_ = gapMessage->endGap();
                    ++gaps_;
                }
            }
        }
   }
}