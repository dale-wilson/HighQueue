// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/ThreadedStepToMessage.hpp>
#include <HighQueue/Consumer.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export InputQueue: public ThreadedStepToMessage
        {
        public:
            InputQueue();

            // Implement ThreadedStepToMessage
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration) override;
            virtual void configureResources(const SharedResourcesPtr & resources) override;
            virtual void attachResources(const SharedResourcesPtr & resources) override;
            virtual void stop() override;
            virtual std::ostream & usage(std::ostream & out) const override;

            virtual void run() override;

        private:
            bool constructWaitStrategy(const ConfigurationNode & config, WaitStrategy & strategy);

        private:
            ConnectionPtr connection_;
            bool discardMessagesIfNoConsumer_;
            CreationParameters parameters_;

            std::unique_ptr<Consumer> consumer_;
//            std::unique_ptr<Message> message_;
        };

    }
}