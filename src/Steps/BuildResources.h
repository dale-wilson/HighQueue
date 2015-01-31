// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "BuildResourcesFwd.h"

#include <Steps/Step_Export.h>

#include <Steps/ComponentBuilderFwd.h>

#include <Steps/ConfigurationFwd.h>
#include <HighQueue/MemoryPoolFwd.h>
#include <HighQueue/ConnectionFwd.h>
#include <Steps/AsioServiceFwd.h>
#include <Steps/StepFwd.h>
#include <HighQueue/WaitStrategyFwd.h>
#include <HighQueue/CreationParametersFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export BuildResources
        {
        public:
            BuildResources();
            ~BuildResources();

            void requestMessages(size_t count = 1);
            void requestMessageSize(size_t bytes);
            void requestAsioThread(size_t count = 1);

            void createResources();

            const MemoryPoolPtr & getMemoryPool()const;
            const AsioServicePtr & getAsioService()const;

            void start();
            void stop();
            void finish();

        public:
            typedef std::map<std::string, QueueBuilderPtr> Queues;

        private:
            /// @brief use a single memory pool for all users.
            /// Note this is a simplification.  If there are dramatic differences in message
            /// sizes there might be an advantage to using multiple pools, but until the need
            /// happens, a single pool will do.
            MemoryPoolPtr pool_;

            /// @brief use a single AsioService for all users.
            AsioServicePtr asio_;

            /// @brief A named collection of Queue Builders
            Queues queues_;

            //////////////////////////
            // Memory Pool Parameters
            size_t numberOfMessagesNeeded_;
            size_t largestMessageSize_;

            //////////////////
            // Asio parameters
            size_t numberOfAsioThreadsNeeded_;

        };
   }
}