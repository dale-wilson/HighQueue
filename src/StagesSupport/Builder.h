// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <StagesSupport/Stage_Export.h>

#include <StagesSupport/ComponentBuilderFwd.h>

#include <StagesSupport/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <StagesSupport/AsioServiceFwd.h>
#include <StagesSupport/StageFwd.h>
#include <HighQueue/WaitStrategyFwd.h>
#include <HighQueue/CreationParametersFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        namespace
        {
            // Top level
            const std::string keyQueue = "queue";
            const std::string keyAsio = "asio";
            const std::string keyPipe = "pipe";
            const std::string keyPool = "memory_pool";

        }

        class Stages_Export Builder
        {
        public:
            Builder();
            ~Builder();

            bool construct(const ConfigurationNodePtr & config);

        public:
            typedef std::map<std::string, PoolBuilderPtr> Pools;
            typedef std::map<std::string, QueueBuilderPtr> Queues;
            typedef std::map<std::string, AsioBuilderPtr> Asios;
            typedef std::vector<StagePtr> Stages;

        private:
            bool constructPipe(const ConfigurationNodePtr & config);

        private:
            Pools pools_;
            Queues queues_;
            Asios asios_;
            Stages stages_;
        };
   }
}