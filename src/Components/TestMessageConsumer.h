// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <HighQueue/Consumer.h>
#include <Mocks/TestMessage.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<size_t Extra = 0>
        class TestMessageConsumer : public std::enable_shared_from_this<TestMessageConsumer<Extra> >
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            TestMessageConsumer(ConnectionPtr & connection, uint32_t messageCount = 0, bool quitOnEmptyMessage = true);
            ~TestMessageConsumer();

            void start();
            void stop();
            void pause();
            void resume();

            uint32_t errors()const
            {
                return sequenceError_;
            }

            void run();
        private:
            ConnectionPtr connection_;
            Consumer consumer_;
            Message message_;
            uint32_t messageCount_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;

            uint32_t sequenceError_;
            
            std::shared_ptr<TestMessageConsumer> me_;
            std::thread thread_;
        };

        template<size_t Extra>
        TestMessageConsumer<Extra>::TestMessageConsumer(ConnectionPtr & connection, uint32_t messageCount, bool quitOnEmptyMessage)
            : connection_(connection)
            , consumer_(connection)
            , message_(connection)
            , messageCount_(messageCount)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , paused_(false)
            , stopping_(false)
            , sequenceError_(0)
        {
        }

        template<size_t Extra>
        TestMessageConsumer<Extra>::~TestMessageConsumer()
        {
            stop();
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::start()
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                TestMessageConsumer<Extra>::run,
                this));
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::stop()
        {
            stopping_ = true;
            if(me_)
            { 
                thread_.join();
                me_.reset();
            }
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::pause()
        {
            paused_ = true;
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::resume()
        {
            paused_ = false;
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::run()
        {
            DebugMessage("Consumer start. " << connection_->getHeader()->name_ << std::endl);
            uint32_t messageCount = 0; 
            uint32_t nextSequence = 0;
            while(!stopping_)
            {
                stopping_ = !consumer_.getNext(message_);
                if(!stopping_ && quitOnEmptyMessage_ && message_.isEmpty())
                {
                    stopping_ = true;
                }
                if(!stopping_)
                { 
                    auto testMessage = message_.get<ActualMessage>();
                    DebugMessage("Consumer: " << testMessage->getSequence() << std::endl);
                    if(nextSequence != testMessage->getSequence())
                    {
                        ++sequenceError_;
                        nextSequence = testMessage->getSequence();
                    }
                    ++nextSequence;
                    message_.destroy<ActualMessage>();
                    ++messageCount;
                    stopping_ = !(messageCount_ == 0 || messageCount < messageCount_);
                }
            }
            DebugMessage("Consumer received " << messageCount << " messages with " << sequenceError_ << " errors" << std::endl);
        }
   }
}