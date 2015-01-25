// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>

#include <StagesSupport/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <StagesSupport/AsioServiceFwd.h>
#include <StagesSupport/StageFwd.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export Builder
        {
        public:
            Builder();
            ~Builder();

            bool construct(const ConfigurationNodePtr & config);

        private:
            bool constructPool(const ConfigurationNodePtr & config);
            bool constructQueue(const ConfigurationNodePtr & config);
            bool constructAsio(const ConfigurationNodePtr & config);
            bool constructPipe(const ConfigurationNodePtr & config);
        private:
            typedef std::map<std::string, MemoryPoolPtr> Pools;
            Pools pools_;
            typedef std::map<std::string, ConnectionPtr> Queues;
            Queues queues_;
            typedef std::map<std::string, AsioServicePtr> Asios;
            Asios asios_;
            typedef std::vector<StagePtr> Stages;
            Stages stages_;
        };
   }
}