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
            TestMessageProducer(
                volatile bool & startSignal, 
                uint32_t messageCount, 
                uint32_t producerNumber);

            bool configure();
            virtual void run();
            void handle(Message & message);

        private:
            volatile bool & startSignal_;
            uint32_t messageCount_;
            uint32_t producerNumber_;
        };

        template<size_t Extra>
        TestMessageProducer<Extra>::TestMessageProducer(
            volatile bool & startSignal, 
            uint32_t messageCount, 
            uint32_t producerNumber)
            : startSignal_(startSignal)
            , messageCount_(messageCount)
            , producerNumber_(producerNumber)
        {
            setName("TestMessageProducer"); // default name
        }

        template<size_t Extra>
        void TestMessageProducer<Extra>::handle(Message & message)
        {
            throw std::runtime_error("TestMessageProducer does not accept incoming Messages");
        }

        template<size_t Extra>
        void TestMessageProducer<Extra>::run()
        {
            outMessage_->setType(Message::TestMessage);
            while(!startSignal_)
            {
                std::this_thread::yield();
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