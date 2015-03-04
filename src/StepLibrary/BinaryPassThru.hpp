// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/StepToMessage.hpp>
#include <HighQueue/Message.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export BinaryPassThru : public StepToMessage
        {
        public:
            explicit BinaryPassThru();

            virtual void handle(Message & message) override;
            virtual void finish() override;
      
        private:
            uint32_t messagesHandled_;
        };
   }
}