// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/ThreadedStageToMessage.h>
#include <Mocks/TestMessage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        template<size_t Extra=0>
        class TestMessageProducer : public ThreadedStageToMessage
        {
        public:
            typedef TestMessage<Extra> ActualMessage;

            static const std::string keyMessageCount;
            static const std::string keyProducerNumber;

            explicit TestMessageProducer(
                volatile bool * startSignal = 0, 
                uint32_t messageCount = 100, 
                uint32_t producerNumber = 1);

            virtual bool configure(ConfigurationNodePtr & config);
            virtual void run();

        private:
            volatile bool * startSignal_;
            uint32_t messageCount_;
            uint32_t producerNumber_;
        };

        template<size_t Extra>
        const std::string TestMessageProducer<Extra>::keyMessageCount = "message_count";
        template<size_t Extra>
        const std::string TestMessageProducer<Extra>::keyProducerNumber = "producer_number";

        template<size_t Extra>
        TestMessageProducer<Extra>::TestMessageProducer(
            volatile bool * startSignal, 
            uint32_t messageCount, 
            uint32_t producerNumber)
            : startSignal_(startSignal)
            , messageCount_(messageCount)
            , producerNumber_(producerNumber)
        {
            setName("TestMessageProducer"); // default name
        }

        template<size_t Extra>
        bool TestMessageProducer<Extra>::configure(ConfigurationNodePtr & config)
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
                        LogFatal("TestMessagProducer can't interpret value for " << keyMessageCount);
                    }
                    messageCount_ = uint32_t(messageCount);
                }
                else if(key == keyProducerNumber)
                {
                    uint64_t producerNumber;
                    if(!config->getValue(producerNumber))
                    {
                        LogFatal("TestMessagProducer can't interpret value for " << keyProducerNumber);
                    }
                    producerNumber_ = uint32_t(producerNumber);
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


        template<size_t Extra>
        void TestMessageProducer<Extra>::run()
        {
            outMessage_->setType(Message::TestMessage);
            if(startSignal_)
            {
                while(!*startSignal_)
                {
                    std::this_thread::yield();
                }
            }
            LogTrace("TestMessageProducer Start ");
            uint32_t messageNumber = 0;
            while(!stopping_ && (messageCount_ == 0 || messageNumber < messageCount_))
            {
                LogVerbose("TestMessageProducer Publish " << messageNumber << '/' << messageCount_);
                auto testMessage = outMessage_->emplace<ActualMessage>(producerNumber_, messageNumber);
                outMessage_->setSequence(messageNumber);
                send(*outMessage_);
                ++messageNumber;
            }
            LogDebug("Producer "<< producerNumber_ <<" publish stop message" );
            outMessage_->setType(Message::Shutdown);
            send(*outMessage_);
            LogInfo("TestMessageProducer " << producerNumber_ << " published " << messageNumber << " messages.");
        }
   }
}