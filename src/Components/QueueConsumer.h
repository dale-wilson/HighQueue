// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/Stage.h>
#include <HighQueue/Consumer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class QueueConsumer: public Stage
        {
        public:
            QueueConsumer();

            // Implement Stage
            virtual void handle(Message & message);

            // todo: virtual void configure(const ConfigurationPtr & configuration);
            void attachConnection(const ConnectionPtr & connection);

            virtual void validate();
            virtual void start();
            virtual void finish();

            void run();
        private:
            void startThread();

        private:
            ConnectionPtr connection_;
            std::unique_ptr<Consumer> consumer_;
            std::unique_ptr<Message> message_;

            std::shared_ptr<Stage> me_;
            std::thread thread_;
        };

    }
}