// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/Stage.h>
#include <Mocks/TestMessage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        template<size_t Extra = 0>
        class TestMessageConsumer : public Stage
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            explicit TestMessageConsumer(uint32_t messageCount_ = 0);

            uint32_t errors()const
            {
                return sequenceError_;
            }

            ////////////////////////////
            // Implement Stage
            virtual void handle(Message & message);

        private:
            uint32_t messageCount_;

            uint32_t messageReceived_;
            uint32_t nextSequence_;
            uint32_t sequenceError_;
        };

        template<size_t Extra>
        TestMessageConsumer<Extra>::TestMessageConsumer(uint32_t messageCount_)
            : messageCount_(messageCount_)
            , messageReceived_(0)
            , nextSequence_(0)
            , sequenceError_(0)
        {
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::handle(Message & message)
        {
            auto type = message.getType();
            switch(type)
            {
                default:
                {
                    LogError("TestMessageConsumer::Expecting test message, not " << Message::toText(type));
                    ++sequenceError_ ;  // define a new error counter for this.
                    return;
                }
                case Message::Shutdown:
                {
                    LogTrace("TestMessageConsumer: received Shutdown");
                    stop();
                    return;
                }
                case Message::TestMessage:
                {
                    auto testMessage = message.get<ActualMessage>();
                    LogDebug("TestMessageConsumer: " << testMessage->getSequence());
                    if(nextSequence_ != testMessage->getSequence())
                    {
                        ++sequenceError_;
                        nextSequence_ = testMessage->getSequence();
                    }
                    ++nextSequence_;
                    message.destroy<ActualMessage>();
                    ++messageReceived_;
                    if(messageCount_ != 0 && messageReceived_ >= messageCount_)
                    {
                        stop();
                    }
                    return;
                }
            }
        }

   }
}