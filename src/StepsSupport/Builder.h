// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StepsSupport/Step_Export.h>
#include <StepsSupport/BuildResources.h>

#include <StepsSupport/ConfigurationFwd.h>
#include <StepsSupport/StepFwd.h>

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
            bool constructPipe(const ConfigurationNode & config);
        private:
            BuildResources resources_;
            Steps Steps_;
        };
   }
}