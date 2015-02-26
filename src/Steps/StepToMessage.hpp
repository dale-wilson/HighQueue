// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.hpp>
#include <HighQueue/MessageFwd.hpp>

#include <Common/Log.hpp>

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
            virtual void configureResources(const SharedResourcesPtr & resources);
            virtual void attachResources(const SharedResourcesPtr & resources);
            virtual void validate();
 
        protected:
            std::unique_ptr<Message> outMessage_;
        };

   }
}