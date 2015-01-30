// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/Stage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export ForwardPassThru: public Stage
        {

        public:
            explicit ForwardPassThru(uint32_t messageCount = 0);

            // implement stage methods
            virtual void handle(Message & message);
 
        private:
            uint32_t messageCount_;
            uint32_t messagesHandled_;
        };
   }
}