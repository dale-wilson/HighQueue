// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.hpp>

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
            virtual void configureResources(const SharedResourcesPtr & resources) override;
            virtual void attachResources(const SharedResourcesPtr & resources) override;
            virtual void validate() override;
 
        protected:
            std::unique_ptr<Message> outMessage_;
            bool destinationIsRequired_;
        };

   }
}