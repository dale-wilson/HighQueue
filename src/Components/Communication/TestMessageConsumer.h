// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Communication/HeaderGenerator.h>
#include <HighQueue/Consumer.h>
#include <Mocks/TestMessage.h>

namespace HighQueue
{
    namespace Communication
    {
        template<size_t Extra=0, typename HeaderGenerator = NullHeaderGenerator>
        class TestMessageConsumer : public std::enable_shared_from_this<TestMessageConsumer<Extra, HeaderGenerator> >
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            TestMessageConsumer(ConnectionPtr & connection, uint32_t messageCount = 0, bool quitOnEmptyMessage);
            ~TestMessageConsumer();

            void start();
            void stop();
            void pause();
            void resume();

        private:
            void runThread(volatile bool & startSignal);
        private:
            HeaderGenerator headerGenerator_;
            ConnectionPtr connection_;
            Consumer consumer_;
            Message message_;
            uint32_t messageCount_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;
            
            std::shared_ptr<TestMessageConsumer> me_;
            std::thread thread_;
        };

        template<size_t Extra, typename HeaderGenerator>
        TestMessageConsumer<Extra, HeaderGenerator>::TestMessageConsumer(ConnectionPtr & connection, uint32_t messageCount, bool quitOnEmptyMessage)
            : connection_(connection)
            , consumer_(connection)
            , message_(connection)
            , messageCount_(0)
            , quitOnEmptyMessage_(quitonEmptyMessage)
            , paused_(false)
            , stopping_(false)
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        TestMessageConsumer<Extra, HeaderGenerator>::~TestMessageConsumer()
        {
            stop();
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageConsumer<Extra, HeaderGenerator>::start()
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                TestMessageConsumer<Extra, HeaderGenerator>::runThread,
                this));
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageConsumer<Extra, HeaderGenerator>::stop()
        {
            stopping_ = true;
            thread_.join();
            me_.reset();
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageConsumer<Extra, HeaderGenerator>::pause()
        {
            // todo
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageConsumer<Extra, HeaderGenerator>::resume()
        {
            // todo
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessageConsumer<Extra, HeaderGenerator>::runThread()
        {
            uint32_t messageNumber = 0; 
            while( messageCount_ > 0 && messageNumber < messageCount_
            {
                messageHeaderGenerator_.addHeader(message_);
                auto testMessage = producerMessage.appendEmplace<ActualMessage>(producerNumber_, messageNumber);
                producer.publish(producerMessage);
                ++messageNumber)
            }
        }
   }
}