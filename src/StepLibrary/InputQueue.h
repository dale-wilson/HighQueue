// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/ThreadedStepToMessage.h>
#include <HighQueue/Consumer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export InputQueue: public ThreadedStepToMessage
        {
        public:
            InputQueue();

            // Implement ThreadedStepToMessage
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(BuildResources & resources);
            virtual void attachResources(BuildResources & resources);
            virtual void run();
            virtual void stop();

            void setStopOnShutdownMessage(bool value);

        private:
            bool constructWaitStrategy(const ConfigurationNode & config, WaitStrategy & strategy);

        private:
            ConnectionPtr connection_;
            bool discardMessagesIfNoConsumer_;
            CreationParameters parameters_;

            std::unique_ptr<Consumer> consumer_;
            std::unique_ptr<Message> message_;
        };

    }
}