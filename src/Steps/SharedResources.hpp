// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "SharedResourcesFwd.hpp"

#include <Steps/Step_Export.hpp>

#include <Steps/ComponentBuilderFwd.hpp>

#include <Steps/ConfigurationFwd.hpp>
#include <HighQueue/MemoryPoolFwd.hpp>
#include <HighQueue/ConnectionFwd.hpp>
#include <Steps/AsioServiceFwd.hpp>
#include <Steps/StepFwd.hpp>
#include <HighQueue/WaitStrategyFwd.hpp>
#include <HighQueue/CreationParametersFwd.hpp>

#include <Common/Log.hpp>
#include <Common/Stopwatch.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export SharedResources : public std::enable_shared_from_this<SharedResources>
        {
        public:
            typedef std::map<std::string, ConnectionPtr> Queues;
            typedef std::vector<StepPtr> Steps;
        public:
            SharedResources();
            ~SharedResources();

            void requestMessages(size_t count = 1);
            void requestMessageSize(size_t bytes);
            void requestAsioThread(size_t threads = 1, size_t tenthsOfThread = 0);

            void addStep(const StepPtr & step);

            void addQueue(const std::string & name, const ConnectionPtr & connection);
            ConnectionPtr findQueue(const std::string & name) const;

            void createResources();

            void attachResources();

            const MemoryPoolPtr & getMemoryPool()const;
            const AsioServicePtr & getAsioService()const;

            const Queues & getQueues()const;
            const Steps & getSteps()const;

            void start();
            void stop();
            void finish();
            void wait();

        private:
            /// @brief use a single memory pool for all users.
            /// Note this is a simplification.  If there are dramatic differences in message
            /// sizes there might be an advantage to using multiple pools, but until the need
            /// happens, a single pool will do.
            MemoryPoolPtr pool_;

            /// @brief use a single AsioService for all users.
            AsioServicePtr asio_;

            /// @brief A named collection of HighQueue Queues
            Queues queues_;

            /// @brief An ordered collection of HighStep Steps (creation order)
            Steps steps_;

            //////////////////////////
            // Memory Pool Parameters
            size_t numberOfMessagesNeeded_;
            size_t largestMessageSize_;

            //////////////////
            // Asio parameters
            size_t tenthsOfAsioThreadsNeeded_;

            Stopwatch timer_;
            uint64_t runTime_;

            bool stopping_;

            std::mutex mutex_;
            std::condition_variable condition_;

        };
   }
}