// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/MessageSource.h>
#include <Mocks/TestMessage.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<size_t Extra=0>
        class TestMessageProducer : public MessageSource
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            TestMessageProducer(
                ConnectionPtr & connection, 
                volatile bool & startSignal, 
                uint32_t messageCount = 0, 
                uint32_t producerNumber = 0, 
                bool sendEmptyMessageOnQuit = true);

            bool configure();
            virtual void run();

        private:
            uint32_t messageCount_;
            uint32_t producerNumber_;
            bool sendEmptyMessageOnQuit_;

            volatile bool & startSignal_;
        };

        template<size_t Extra>
        TestMessageProducer<Extra>::TestMessageProducer(
            ConnectionPtr & connection, 
            volatile bool & startSignal, 
            uint32_t messageCount, 
            uint32_t producerNumber, 
            bool sendEmptyMessageOnQuit)
            : MessageSource(connection)
            , startSignal_(startSignal)
            , messageCount_(messageCount)
            , producerNumber_(producerNumber)
            , sendEmptyMessageOnQuit_(sendEmptyMessageOnQuit)
        {
            outMessage_.meta().type_ = Message::Meta::TestMessage;
        }

        template<size_t Extra>
        bool TestMessageProducer<Extra>::configure()
        {
            // todo
            return true;
        }

        template<size_t Extra>
        void TestMessageProducer<Extra>::run()
        {
            while(!startSignal_)
            {
                std::this_thread::yield();
            }
            DebugMessage("Producer Start " << connection_->getHeader()->name_ << std::endl);
            uint32_t messageNumber = 0;
            while( messageCount_ == 0 || messageNumber < messageCount_)
            {
                // DebugMessage("Publish " << messageNumber << '/' << messageCount_ << std::endl);
                auto testMessage = outMessage_.emplace<ActualMessage>(producerNumber_, messageNumber);
                producer_.publish(outMessage_);
                ++messageNumber;
            }
            if(sendEmptyMessageOnQuit_)
            {
                DebugMessage("Producer "<< producerNumber_ <<"publish empty message" << std::endl);
                producer_.publish(outMessage_);
            }
            DebugMessage("Producer " << producerNumber_ << " published " << messageNumber << " messages." << std::endl);
        }
   }
}