// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/MessageSink.h>
#include <Mocks/TestMessage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        template<size_t Extra = 0>
        class TestMessageConsumer : public MessageSink
        {
        public:
            typedef TestMessage<Extra> ActualMessage;
            TestMessageConsumer(ConnectionPtr & connection, uint32_t messageCount_ = 0, bool quitOnEmptyMessage = true);

            uint32_t errors()const
            {
                return sequenceError_;
            }

            ////////////////////////////
            // Implement MessageSink
            virtual void handleMessageType(Message::Meta::MessageType type, Message & message);

        private:
            uint32_t messageCount_;
            bool quitOnEmptyMessage_;

            uint32_t messageReceived_;
            uint32_t nextSequence_;
            uint32_t sequenceError_;
        };

        template<size_t Extra>
        TestMessageConsumer<Extra>::TestMessageConsumer(ConnectionPtr & connection, uint32_t messageCount_, bool quitOnEmptyMessage)
            : MessageSink(connection)
            , messageCount_(messageCount_)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , messageReceived_(0)
            , nextSequence_(0)
            , sequenceError_(0)
        {
        }

        template<size_t Extra>
        void TestMessageConsumer<Extra>::handleMessageType(Message::Meta::MessageType type, Message & message)
        {
            if(type != Message::Meta::TestMessage)
            {
                LogError("TestMessageConsumer::Expecting test message, not " << Message::Meta::toText(type));
                return;
            }
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
        }

   }
}