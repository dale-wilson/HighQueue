// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StageCommon/Stage_Export.h>

#include <StageCommon/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <StageCommon/AsioServiceFwd.h>
#include <StageCommon/StageFwd.h>

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
            bool constructPools(const ConfigurationNodePtr & config);
            bool constructQueues(const ConfigurationNodePtr & config);
            bool constructAsios(const ConfigurationNodePtr & config);
            bool constructPipes(const ConfigurationNodePtr & config);
        private:
            typedef std::map<std::string, MemoryPoolPtr> pools_;
            typedef std::map<std::string, ConnectionPtr> queues_;
            typedef std::map<std::string, AsioServicePtr> asios_;
            typedef std::vector<StagePtr> stages_;
        };
   }
}