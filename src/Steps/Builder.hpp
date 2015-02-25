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
            bool configureParameter(const StepPtr & step, const std::string & key, const ConfigurationNode & configuration);

        private:
            SharedResources resources_;
            Steps Steps_;
        };
   }
}