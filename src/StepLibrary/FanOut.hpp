// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/StepToMessage.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export FanOut: public StepToMessage
        {
        public:
            explicit FanOut();

            // implement Step methods
            virtual void handle(Message & message) override;
            virtual void logStats() override;
 
        private:
            uint32_t messagesHandled_;
            uint32_t messagesSent_;
        };
   }
}