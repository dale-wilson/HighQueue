// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StepsSupport/Step.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export ForwardPassThru: public Step
        {

        public:
            explicit ForwardPassThru(uint32_t messageCount = 0);

            // implement Step methods
            virtual void handle(Message & message);
 
        private:
            uint32_t messageCount_;
            uint32_t messagesHandled_;
        };
   }
}