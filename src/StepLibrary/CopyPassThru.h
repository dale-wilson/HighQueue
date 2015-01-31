// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/StepToMessage.h>
#include <HighQueue/Message.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        template<typename CargoType>
        class CopyPassThru : public StepToMessage
        {

        public:
            explicit CopyPassThru(uint32_t messageCount = 0);

            virtual void handle(Message & message);
 
        private:
            uint32_t messageCount_;
            uint32_t messagesHandled_;
        };

        template<typename CargoType>
        CopyPassThru<CargoType>::CopyPassThru(uint32_t messageCount)
            : messageCount_(messageCount)
            , messagesHandled_(0)
        {
            setName("CopyPassThru"); // default name
        }

        template<typename CargoType>
        void CopyPassThru<CargoType>::handle(Message & message)
        {
            if(!stopping_)
            { 
                LogDebug("CopyPassThru<CargoType> copy.");
                outMessage_->emplace<CargoType>(*message.get<CargoType>());
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