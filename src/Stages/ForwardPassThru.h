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
        class ForwardPassThru: public Stage
        {

        public:
            explicit ForwardPassThru(uint32_t messageCount = 0);

            // implement stage methods
            virtual void handle(Message & message);
 
        private:
            uint32_t messageCount_;
            uint32_t messagesHandled_;
        };

		inline
        ForwardPassThru::ForwardPassThru(uint32_t messageCount)
            : messageCount_(messageCount)
            , messagesHandled_(0)
        {
            setName("ForwardPassThru"); // default name
        }

		inline
        void ForwardPassThru::handle(Message & message)
        {
            if(!stopping_)
            { 
                LogTrace("ForwardPassThru copy.");
				send(message);
                ++messagesHandled_;
                if(messageCount_ != 0 && messagesHandled_ >= messageCount_)
                {
                    LogTrace("ForwardPassThru stop: message count: " << messagesHandled_);
                    stop();
                }
            }
        }
   }
}