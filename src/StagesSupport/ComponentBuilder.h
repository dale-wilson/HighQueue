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

        public:
            ComponentBuilder();
            virtual ~ComponentBuilder();

            bool configure(const ConfigurationNodePtr & config);
            const std::string & getName()
            {
                return name_;
            }
        protected:
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter) = 0;
            virtual bool validate() = 0;
            virtual void create() = 0;

        protected:
            std::string name_;
        };

        template<typename Component>
        class TypedComponentBuilder : public ComponentBuilder
        {
        public:
            typedef std::shared_ptr<Component> ComponentPtr;
            TypedComponentBuilder()
            {}

            virtual ~TypedComponentBuilder()
            {}

            const ComponentPtr & get() const
            {
                return value_;
            }
        protected:
            ComponentPtr value_;
        };

        class PoolBuilder: public TypedComponentBuilder < MemoryPool >
        {
        public:
            PoolBuilder();
            virtual ~PoolBuilder();
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter);
            virtual bool validate();
            virtual void create();

            void addToMessageCount(size_t additionalMessages);
            void needAtLeast(size_t byteCount);
        private:
            static const size_t NONE = ~size_t(0);
            uint64_t messageSize_ = NONE;
            uint64_t messageCount_ = NONE;
        };

        class AsioBuilder: public TypedComponentBuilder<AsioService>
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

        class QueueBuilder: public TypedComponentBuilder<Connection>
        {
        public:
            QueueBuilder(Builder::Pools & pools);
            virtual ~QueueBuilder();
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter);
            virtual bool validate();
            virtual void create();

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

        class PipeBuilder: public ComponentBuilder
        {
        public:
            PipeBuilder(Builder::Stages & stages, Builder::Pools & pools, Builder::Asios & asios, Builder::Queues & queues);
            virtual ~PipeBuilder();
            virtual bool interpretParameter(const std::string & key, ConfigurationNodePtr & parameter);
            virtual bool validate();
            virtual void create();

        private:
            Builder::Stages & stages_;
            Builder::Pools & pools_;
            Builder::Asios & asios_;
            Builder::Queues & queues_;

        };
   }
}