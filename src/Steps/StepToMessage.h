// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.h>
#include <HighQueue/MessageFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class  Steps_Export StepToMessage : public Step
        {

        public:
            StepToMessage();
            virtual ~StepToMessage();

            // implement Step methods
            virtual void configureResources(BuildResources & resources);
            virtual void attachResources(BuildResources & resources);
            virtual void validate();
 
        protected:
            std::unique_ptr<Message> outMessage_;
        };

   }
}