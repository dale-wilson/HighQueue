// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/ThreadedStepToMessage.hpp>
#include <Mocks/MockMessage.hpp>
#include <Steps/Configuration.hpp>
#include <Steps/SharedResources.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export  BaseMessageProducer : public ThreadedStepToMessage
        {
        public:
            BaseMessageProducer();
            virtual ~BaseMessageProducer();
            /// @brief Support starting several message producers "at the same time"
            /// @brief startSignal should be false initially then become true when it's time to begin producing messages
            void setStartSignal(volatile bool * startSignal);

            virtual std::ostream & usage(std::ostream & out) const override;
            virtual bool configureParameter(const std::string & name,  const ConfigurationNode & config) override;
            virtual void validate() override;
            virtual void logStats() override;

        protected:
            volatile bool * startSignal_;
            uint32_t messageCount_;
            uint32_t producerNumber_;
            uint32_t messageNumber_;
        };

        template<typename MockMessageType>
        class MockMessageProducer : public BaseMessageProducer
        {
        public:
            typedef MockMessageType ActualMessage;

            MockMessageProducer();

            virtual void configureResources(const SharedResourcesPtr & resources) override;
            virtual void run() override;
        };

        template<typename MockMessageType>
        MockMessageProducer<MockMessageType>::MockMessageProducer()
        {
        }

        template<typename MockMessageType>
        void MockMessageProducer<MockMessageType>::configureResources(const SharedResourcesPtr & resources)
        {
            resources->requestMessageSize(sizeof(MockMessageType));
            BaseMessageProducer::configureResources(resources);
        }

        template<typename MockMessageType>
        void MockMessageProducer<MockMessageType>::run()
        {
            outMessage_->setType(Message::MessageType::MockMessage);
            if(startSignal_)
            {
                while(!*startSignal_)
                {
                    std::this_thread::yield();
                }
            }
            LogTrace("MockMessageProducer Start  " << name_);
            messageNumber_ = 0;
            while(!stopping_ && (messageCount_ == 0 || messageNumber_ < messageCount_))
            {
                LogVerbose("MockMessageProducer  " << name_ << " Publish " << messageNumber_ << '/' << messageCount_);
                auto testMessage = outMessage_->emplace<ActualMessage>(producerNumber_, messageNumber_);
                outMessage_->setSequence(messageNumber_);
                send(*outMessage_);
                ++messageNumber_;
            }
            LogDebug("Producer  " << name_ << " " << producerNumber_ << " publish stop message");
            outMessage_->setType(Message::MessageType::Shutdown);
            send(*outMessage_);
        }
   }
}