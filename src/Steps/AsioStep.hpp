// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.hpp>
#include <Steps/AsioService.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class  Steps_Export AsioStep : public Step
        {

        public:
            AsioStep();
            virtual ~AsioStep();
            virtual void configureResources(SharedResources & resources);
            virtual void attachResources(SharedResources & resources);
            virtual void validate();

            virtual void start();
            virtual void finish();
        protected:
            AsioServicePtr ioService_;
            std::shared_ptr<Step> me_;
        };
   }
}