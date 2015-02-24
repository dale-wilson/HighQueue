// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export ForwardPassThru: public Step
        {
        public:
            explicit ForwardPassThru();

            // implement Step methods
            virtual void handle(Message & message);
            virtual void finish();

        private:
            uint32_t messagesHandled_;
        };
   }
}