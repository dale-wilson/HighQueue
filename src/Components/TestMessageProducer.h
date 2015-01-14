// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/HeaderGenerator.h>
#include <HighQueue/Producer.h>
#include <Mocks/TestMessage.h>
#define USE_DEBUG_MESSAGE 0
#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<size_t Extra=0, typename HeaderGenerator = NullHeaderGenerator>
        class TestMessageProducer : public std::enable_shared_from_this<TestMessageProducer<Extra, HeaderGenerator> >
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            TestMessageProducer(ConnectionPtr & connection, uint32_t messageCount = 0, uint32_t producerNumber = 0, bool sendEmptyMessageOnQuit = true);
            ~TestMessageProducer();

            bool configure();
            void start(volatile bool & startSignal);
            void stop();
            void pause();
            void resume();

        private:
            void runThread(volatile bool & startSignal);
        private:
            HeaderGenerator headerGenerator_;
            ConnectionPtr connection_;
            Producer producer_;
            Message message_;
            uint32_t messageCount_;
            uint32_t producerNumber_;
            bool sendEmptyMessageOnQuit_;
            bool paused_;
            bool stopping_;

            std::shared_ptr<TestMessageProducer> me_;
            std::thread thread_;
        };

        template<size_t Extra, typename HeaderGenerator>
        TestMessageProducer<Extra, HeaderGenerator>::TestMessageProducer(ConnectionPtr & connection, uint32_t messageCount, uint32_t producerNumber, bool sendEmptyMessageOnQuit)
            : connection_(connection)
            , producer_(connection)
            , message_(connection)
            , messageCount_(messageCount)
            , producerNumber_(producerNumber)
            , sendEmptyMessageOnQuit_(sendEmptyMessageOnQuit)
            , paused_(false)
            , stopping_(false)
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        TestMessageProducer<Extra, HeaderGenerator>::~TestMessageProducer()
        {
            stop();
        }

        template<size_t Extra, typename HeaderGenerator>
        bool TestMessageProducer<Extra, HeaderGenerator>::configure()
        {
            // todo
            return true;
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageProducer<Extra, HeaderGenerator>::start(volatile bool & startSignal)
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                &TestMessageProducer<Extra, HeaderGenerator>::runThread,
                this, std::ref(startSignal)));
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageProducer<Extra, HeaderGenerator>::stop()
        {
            stopping_ = true;
            if(me_)
            {
                thread_.join();
                me_.reset();
            }
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageProducer<Extra, HeaderGenerator>::pause()
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageProducer<Extra, HeaderGenerator>::resume()
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageProducer<Extra, HeaderGenerator>::runThread(volatile bool & startSignal)
        {
            while(!startSignal)
            {
                std::this_thread::yield();
            }
            DebugMessage("Producer Start " << connection_->getHeader()->name_ << std::endl);
            uint32_t messageNumber = 0;
            while( messageCount_ == 0 || messageNumber < messageCount_)
            {
                // DebugMessage("Publish " << messageNumber << '/' << messageCount_ << std::endl);
                headerGenerator_.addHeader(message_);
                auto testMessage = message_.appendEmplace<ActualMessage>(producerNumber_, messageNumber);
                producer_.publish(message_);
                ++messageNumber;
            }
            if(sendEmptyMessageOnQuit_)
            {
                producer_.publish(message_);
            }
            DebugMessage("Producer " << producerNumber_ << " published " << messageNumber << " messages." << std::endl);
        }
   }
}