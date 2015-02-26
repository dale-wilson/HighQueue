// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Step_Export.hpp>
#include <Steps/SharedResources.hpp>

#include <Steps/ConfigurationFwd.hpp>
#include <Steps/StepFwd.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export Builder
        {
        public:
            Builder();
            ~Builder();

            bool construct(const ConfigurationNode & config);
            void start();
            void stop();
            void wait();
            void finish();

        private:
            bool constructPipe(const ConfigurationNode & config, const StepPtr & parentStep);
            bool configureParameter(const StepPtr & step, const std::string & key, const ConfigurationNode & configuration);

        private:
            SharedResourcesPtr resources_;
        };
   }
}