// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/ThreadedStageToMessage.h>
#include <Mocks/MockMessage.h>
#include <StagesSupport/Configuration.h>
#include <StagesSupport/BuildResources.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        template<typename MockMessageType>
        class MockMessageProducer : public ThreadedStageToMessage
        {
        public:
            typedef MockMessageType ActualMessage;

            static const std::string keyMessageCount;
            static const std::string keyProducerNumber;

            MockMessageProducer();

            void setStartSignal(volatile bool * startSignal)
            {
                startSignal_ = startSignal;
            }

            virtual bool configureParameter(const std::string & name,  const ConfigurationNode & config);
            virtual void configureResources(BuildResources & resources);
            virtual void validate();
            virtual void run();

        private:
            volatile bool * startSignal_;
            uint32_t messageCount_;
            uint32_t producerNumber_;
        };

        template<typename MockMessageType>
        const std::string MockMessageProducer<MockMessageType>::keyMessageCount = "message_count";
        template<typename MockMessageType>
        const std::string MockMessageProducer<MockMessageType>::keyProducerNumber = "producer_number";

        template<typename MockMessageType>
        MockMessageProducer<MockMessageType>::MockMessageProducer()
            : startSignal_(0)
            , messageCount_(~uint32_t(0))
            , producerNumber_(0)
        {
        }

        template<typename MockMessageType>
        bool MockMessageProducer<MockMessageType>::configureParameter(const std::string & key, const ConfigurationNode & config)
        {
            if(key == keyMessageCount)
            {
                uint64_t messageCount;
                if(!config.getValue(messageCount))
                {
                    LogFatal("MockMessageProducer can't interpret value for " << keyMessageCount);
                    return false;
                }
                messageCount_ = uint32_t(messageCount);
            }
            else if(key == keyProducerNumber)
            {
                uint64_t producerNumber;
                if(!config.getValue(producerNumber))
                {
                    LogFatal("TestMessagProducer can't interpret value for " << keyProducerNumber);
                    return false;
                }
                producerNumber_ = uint32_t(producerNumber);
            }
            else
            {
                return ThreadedStageToMessage::configureParameter(key, config);
            }
            return true;
        }

        template<typename MockMessageType>
        void MockMessageProducer<MockMessageType>::configureResources(BuildResources & resources)
        {
            resources.requestMessageSize(sizeof(MockMessageType));
            ThreadedStageToMessage::configureResources(resources);

        }
        template<typename MockMessageType>
        void MockMessageProducer<MockMessageType>::validate()
        {
            ThreadedStageToMessage::validate();
            if(messageCount_ == ~uint32_t(0))
            {
                std::stringstream msg;
                msg << "MockMessageProducer " << name_ << " missing \"" << keyMessageCount << "\" parameter.";
                throw std::runtime_error(msg.str());
            }
        }

        template<typename MockMessageType>
        void MockMessageProducer<MockMessageType>::run()
        {
            outMessage_->setType(Message::MockMessage);
            if(startSignal_)
            {
                while(!*startSignal_)
                {
                    std::this_thread::yield();
                }
            }
            LogTrace("MockMessageProducer Start ");
            uint32_t messageNumber = 0;
            while(!stopping_ && (messageCount_ == 0 || messageNumber < messageCount_))
            {
                LogVerbose("MockMessageProducer Publish " << messageNumber << '/' << messageCount_);
                auto testMessage = outMessage_->emplace<ActualMessage>(producerNumber_, messageNumber);
                outMessage_->setSequence(messageNumber);
                send(*outMessage_);
                ++messageNumber;
            }
            LogDebug("Producer "<< producerNumber_ <<" publish stop message" );
            outMessage_->setType(Message::Shutdown);
            send(*outMessage_);
            LogInfo("MockMessageProducer " << producerNumber_ << " published " << messageNumber << " messages.");
        }
   }
}