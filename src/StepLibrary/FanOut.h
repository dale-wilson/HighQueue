// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/StepToMessage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export FanOut: public StepToMessage
        {
        public:
            explicit FanOut();

            // implement Step methods
            virtual void handle(Message & message);
            virtual void finish();
 
        private:
            uint32_t messagesHandled_;
        };
   }
}