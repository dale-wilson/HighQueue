// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/StageToMessage.h>
#include <HighQueue/Message.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export BinaryPassThru : public StageToMessage
        {

        public:
            explicit BinaryPassThru(uint32_t messageCount = 0);

            virtual void handle(Message & message);
 
        private:
            uint32_t messageCount_;
            uint32_t messagesHandled_;
        };
   }
}