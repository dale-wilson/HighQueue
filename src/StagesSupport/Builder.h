// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>
#include <StagesSupport/BuildResources.h>

#include <StagesSupport/ConfigurationFwd.h>
#include <StagesSupport/StageFwd.h>

namespace HighQueue
{
    namespace Stages
    {
        namespace
        {
            // Top level
            const std::string keyPipe = "pipe";
        }

        class Stages_Export Builder
        {
        public:
            Builder();
            ~Builder();

            bool construct(const ConfigurationNode & config);
            void start();
            void stop();
            void finish();

        public:
            typedef std::vector<StagePtr> Stages;

        private:
            bool constructPipe(const ConfigurationNode & config);
        private:
            BuildResources resources_;
            Stages stages_;
        };
   }
}