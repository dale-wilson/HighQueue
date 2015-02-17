// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <Steps/Step_Export.h>
#include <Steps/SharedResources.h>

#include <Steps/ConfigurationFwd.h>
#include <Steps/StepFwd.h>

namespace HighQueue
{
    namespace Steps
    {
        namespace
        {
            // Top level
            const std::string keyPipe = "pipe";
        }

        class Steps_Export Builder
        {
        public:
            Builder();
            ~Builder();

            bool construct(const ConfigurationNode & config);
            void start();
            void stop();
            void finish();

        public:
            typedef std::vector<StepPtr> Steps;

        private:
            bool constructPipe(const ConfigurationNode & config, const StepPtr & parentStep);
        private:
            SharedResources resources_;
            Steps Steps_;
        };
   }
}