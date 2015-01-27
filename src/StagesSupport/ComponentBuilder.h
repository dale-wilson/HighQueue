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
#include <HighQueue/WaitStrategyFwd.h>
#include <HighQueue/WaitStrategy.h>

#include <HighQueue/CreationParametersFwd.h>

#include <StagesSupport/Builder.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        template<typename Component>
        class ComponentBuilder
        {
        public:
            static const std::string keyName;
            // todo move these into appropriate builders
            static const std::string keyMessageSize;
            static const std::string keyMessageCount;

            static const std::string keyConsumerWaitStrategy;
            static const std::string keyProducerWaitStrategy;
            static const std::string keyCommonWaitStrategy;

            static const std::string keySpinCount;
            static const std::string keyYieldCount;
            static const std::string keySleepCount;
            static const std::string keySleepPeriod;
            static const std::string keyMutexWaitTimeout;

            static const std::string valueForever;

            static const std::string keyPool;
            static const std::string keyDiscardMessagesIfNoConsumer;
            static const std::string keyEntryCount;

            static const std::string keyThreadCount;

            typedef std::shared_ptr<Component> ComponentPtr;
            ComponentBuilder();

            virtual ~ComponentBuilder();
            bool configure(const ConfigurationNodePtr & config);
            const std::string & getName()
            {
                return name_;
            }

            const ComponentPtr & get() const
            {
                return value_;
            }

        protected:
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter) = 0;
            virtual bool validate() = 0;
            virtual void create() = 0;

        protected:
            std::string name_;
            ComponentPtr value_;
        };

        template<typename Component>
        const std::string ComponentBuilder<Component>::keyName = "name";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyMessageSize = "message_size";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyMessageCount = "message_count";

        template<typename Component>
        const std::string ComponentBuilder<Component>::keyConsumerWaitStrategy = "consumer_wait_strategy";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyProducerWaitStrategy = "produder_wait_strategy";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyCommonWaitStrategy = "common_wait_strategy";

        template<typename Component>
        const std::string ComponentBuilder<Component>::keySpinCount = "spin_count";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyYieldCount = "yield_count";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keySleepCount = "sleep_count";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keySleepPeriod = "sleep_nanoseconds";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyMutexWaitTimeout = "timeout_nanoseconds";

        template<typename Component>
        const std::string ComponentBuilder<Component>::valueForever = "forever";

        template<typename Component>
        const std::string ComponentBuilder<Component>::keyPool = "memory_pool";

        template<typename Component>
        const std::string ComponentBuilder<Component>::keyDiscardMessagesIfNoConsumer = "discard_messages_if_no_consumer";
        template<typename Component>
        const std::string ComponentBuilder<Component>::keyEntryCount = "entry_count";

        template<typename Component>
        const std::string ComponentBuilder<Component>::keyThreadCount = "thread_count";

        template<typename Component>
        ComponentBuilder<Component>::ComponentBuilder()
        {}

        template<typename Component>
        ComponentBuilder<Component>::~ComponentBuilder()
        {
        }

        template<typename Component>
        bool ComponentBuilder<Component>::configure(const ConfigurationNodePtr & config)
        {
            for(auto poolChildren = config->getChildren();
                poolChildren->has();
                poolChildren->next())
            {
                auto & parameter = poolChildren->getChild();
                auto & key = parameter->getName();

                if(key == keyName)
                {
                    parameter->getValue(name_);
                }
                else if(! interpretParameter(key, parameter))
                {
                    return false;
                }
            }

            if(name_.empty())
            {
                LogFatal("Missing required parameter " << keyName << " for " << keyAsio << ".");
                return false;
            }
            return true;
        }


        class PoolBuilder: public ComponentBuilder < MemoryPool >
        {
        public:
            PoolBuilder();
            virtual ~PoolBuilder();
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter);
            virtual bool validate();
            virtual void create();
        private:
            static const size_t NONE = ~size_t(0);
            uint64_t messageSize_ = NONE;
            uint64_t messageCount_ = NONE;
        };

        class AsioBuilder : public ComponentBuilder<AsioService>
        {
        public:
            AsioBuilder();
            virtual ~AsioBuilder();
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter);
            virtual bool validate();
            virtual void create();

        private:
            size_t threadCount_;
        };

        class QueueBuilder: public ComponentBuilder<Connection>
        {
        public:
            QueueBuilder(Builder::Pools & pools);
            virtual ~QueueBuilder();
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter);
            virtual bool validate();
            virtual void create();
            virtual void start();


            bool constructWaitStrategy(const ConfigurationNodePtr & config, WaitStrategy & strategy);
            bool constructCreationParameters(const ConfigurationNodePtr & config, CreationParameters & parameters);


        private:
            Builder::Pools & pools_;

            std::string poolName_;

            WaitStrategy producerWaitStrategy_;
            WaitStrategy consumerWaitStrategy_;
            bool discardMessagesIfNoConsumer_;
            uint64_t entryCount_;
            uint64_t messageSize_;
            uint64_t messageCount_;
        };
   }
}