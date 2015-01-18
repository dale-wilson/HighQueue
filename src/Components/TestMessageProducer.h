// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/MessageSource.h>
#include <Mocks/TestMessage.h>

#include <Common/Log.h>

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
                uint32_t producerNumber = 0);

            bool configure();
            virtual void run();

        private:
            uint32_t messageCount_;
            uint32_t producerNumber_;
            volatile bool & startSignal_;
        };

        template<size_t Extra>
        TestMessageProducer<Extra>::TestMessageProducer(
            ConnectionPtr & connection, 
            volatile bool & startSignal, 
            uint32_t messageCount, 
            uint32_t producerNumber)
            : MessageSource(connection)
            , startSignal_(startSignal)
            , messageCount_(messageCount)
            , producerNumber_(producerNumber)
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
            LogTrace("TestMessageProducer Start " << outConnection_->getHeader()->name_);
            uint32_t messageNumber = 0;
            while(!stopping_ && messageCount_ == 0 || messageNumber < messageCount_)
            {
                LogVerbose("TestMessageProducer Publish " << messageNumber << '/' << messageCount_);
                auto testMessage = outMessage_.emplace<ActualMessage>(producerNumber_, messageNumber);
                publish(outMessage_);
                ++messageNumber;
            }
            LogDebug("Producer "<< producerNumber_ <<" publish stop message" );
            outMessage_.meta().type_ = Message::Meta::Shutdown;
            publish(outMessage_);
            LogInfo("TestMessageProducer " << producerNumber_ << " published " << messageNumber << " messages.");
        }
   }
}