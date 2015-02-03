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
        class Steps_Export BinaryPassThru : public StepToMessage
        {

        public:
            explicit BinaryPassThru();

            virtual void handle(Message & message);
            virtual void finish();
 
        private:
            uint32_t messagesHandled_;
        };
   }
}