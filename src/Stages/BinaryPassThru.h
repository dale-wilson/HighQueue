// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/StageToMessage.h>
#include <HighQueue/Message.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class BinaryPassThru : public StageToMessage
        {

        public:
            explicit BinaryPassThru(uint32_t messageCount = 0);

            virtual void handle(Message & message);
 
        private:
            uint32_t messageCount_;
            uint32_t messagesHandled_;
        };

        BinaryPassThru::BinaryPassThru(uint32_t messageCount)
            : messageCount_(messageCount)
            , messagesHandled_(0)
        {
            setName("BinaryPassThru"); // default name
        }
        
        void BinaryPassThru::handle(Message & message)
        {
            if(!stopping_)
            { 
                LogDebug("BinaryPassThru copy.");
                outMessage_->appendBinaryCopy(message.get(), message.getUsed());
				message.moveMetaInfoTo(*outMessage_);
                send(*outMessage_);
                auto type = message.getType();
                if(type == Message::MessageType::Shutdown)
                {
                    stop();
                }
                ++messagesHandled_;
                if(messageCount_ != 0 && messagesHandled_ >= messageCount_)
                {
                    stop();
                }
            }
        }
   }
}