// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Communication/HeaderGenerator.h>
#include <HighQueue/Producer.h>
#include <Mocks/TestMessage.h>

namespace HighQueue
{
    namespace Communication
    {
        template<size_t Extra=0, typename HeaderGenerator = NullHeaderGenerator>
        class TestMessagePublisher : public std::enable_shared_from_this<TestMessagePublisher<Extra, HeaderGenerator> >
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            TestMessagePublisher(ConnectionPtr & connection, uint32_t messageCount = 0, uint32_t producerNumber = 0);
            ~TestMessagePublisher();

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
            bool paused_;
            bool stopping_;

            std::shared_ptr<TestMessagePublisher> me_;
            std::thread thread_;
        };

        template<size_t Extra, typename HeaderGenerator>
        TestMessagePublisher<Extra, HeaderGenerator>::TestMessagePublisher(ConnectionPtr & connection, uint32_t messageCount, uint32_t producerNumber)
            : connection_(connection)
            , producer_(connection)
            , message_(connection)
            , messageCount_(0)
            , producerNumber_(producerNumber)
            , paused_(false)
            , stopping_(false)
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        TestMessagePublisher<Extra, HeaderGenerator>::~TestMessagePublisher()
        {
            stop();
        }

        template<size_t Extra, typename HeaderGenerator>
        bool TestMessagePublisher<Extra, HeaderGenerator>::configure()
        {
            // todo
            return true;
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessagePublisher<Extra, HeaderGenerator>::start(volatile bool & startSignal)
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                TestMessagePublisher<Extra, HeaderGenerator>::runThread,
                this, std::ref(startSignal)));
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessagePublisher<Extra, HeaderGenerator>::stop()
        {
            stopping_ = true;
            thread_.join();
            me_.reset();
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessagePublisher<Extra, HeaderGenerator>::pause()
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessagePublisher<Extra, HeaderGenerator>::resume()
        {
        }

        template<size_t Extra, typename HeaderGenerator>
        void TestMessagePublisher<Extra, HeaderGenerator>::runThread(volatile bool & startSignal)
        {
            while(!startSignal)
            {
                std::this_thread_yield();
            }
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