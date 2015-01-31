// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/Stage.h>
#include <Mocks/MockMessage.h>
#include <StagesSupport/Configuration.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        template<typename MockMessageType>
        class MockMessageConsumer : public Stage
        {
        public:
            typedef MockMessageType ActualMessage;
            static const std::string keyMessageCount;

            explicit MockMessageConsumer(uint32_t messageCount_ = 0);

            virtual bool configure(ConfigurationNodePtr & config);

            uint32_t errors()const
            {
                return sequenceError_ + unexpectedMessageError_;
            }
            uint32_t messagesHandled()const
            {
                return messagesHandled_;
            }

            ////////////////////////////
            // Implement Stage
            virtual void handle(Message & message);

        private:
            uint32_t messageCount_;

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
            , messagesHandled_(0)
            , nextSequence_(0)
            , sequenceError_(0)
            , unexpectedMessageError_(0)
        {
            setName("MockMessageConsumer"); // default name
        }


        template<typename MockMessageType>
        bool MockMessageConsumer<MockMessageType>::configure(ConfigurationNodePtr & config)
        {
            for(auto poolChildren = config->getChildren();
                poolChildren->has();
                poolChildren->next())
            {
                auto & parameter = poolChildren->getChild();
                auto & key = parameter->getName();

                if(key == keyName)
                {
                    parameter->getValue(name_);
                }
                else if(key == keyMessageCount)
                {
                    uint64_t messageCount;
                    if(!config->getValue(messageCount))
                    {
                        LogFatal("TestMessagConsumer can't interpret value for " << keyMessageCount);
                    }
                    messageCount_ = uint32_t(messageCount);
                }
                else
                {
                    LogFatal("Unknown configuration parameter " << key << "  " << config->getName() << " "  << name_);
                    return false;
                }
            }
            
            if(name_.empty())
            {
                LogFatal("Missing required parameter " << keyName << " for  " << config->getName() << ".");
                return false;
            }
            return true;
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
            }
        }

   }
}