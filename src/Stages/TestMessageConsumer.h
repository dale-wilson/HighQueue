// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/Stage.h>
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
                return sequenceError_ + unexpectedMessageError_;
            }
			uint32_t messagesHandled()const
			{
				return messagesHandled_;
			}

            ////////////////////////////
            // Implement Stage
            virtual void handle(Message & message);

        private:
            uint32_t messageCount_;

            uint32_t messagesHandled_;
            uint32_t nextSequence_;
            uint32_t sequenceError_;
			uint32_t unexpectedMessageError_;
        };

        template<size_t Extra>
        TestMessageConsumer<Extra>::TestMessageConsumer(uint32_t messageCount_)
            : messageCount_(messageCount_)
            , messagesHandled_(0)
            , nextSequence_(0)
            , sequenceError_(0)
			, unexpectedMessageError_(0)
        {
            setName("TestMessageConsumer"); // default name
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
					++unexpectedMessageError_;  
					return;
                }
                case Message::Shutdown:
                {
					if (messageCount_ == 0)
					{
						LogTrace("TestMessageConsumer: received Shutdown");
						stop();
					}
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
                    ++messagesHandled_;
                    if(messageCount_ != 0 && messagesHandled_ >= messageCount_)
                    {
                        stop();
                    }
                    return;
                }
            }
        }

   }
}