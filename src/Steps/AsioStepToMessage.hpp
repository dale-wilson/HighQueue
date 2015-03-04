// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/StepToMessage.hpp>
#include <Steps/AsioService.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class  Steps_Export AsioStepToMessage : public StepToMessage
        {

        public:
            AsioStepToMessage();
            virtual ~AsioStepToMessage();
            virtual void configureResources(const SharedResourcesPtr & resources) override;
            virtual void attachResources(const SharedResourcesPtr & resources) override;
            virtual void validate() override;

            virtual void start() override;
            virtual void finish() override;
        protected:
            AsioServicePtr ioService_;
            std::shared_ptr<Step> me_;
        };
   }
}