// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export MockMessageAnalyzer: public Step
        {
        public:
            explicit MockMessageAnalyzer();

            // implement Step methods
            virtual void handle(Message & message);
            virtual void finish();

        private:
            uint32_t messagesHandled_;
        };
   }
}