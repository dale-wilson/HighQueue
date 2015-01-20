// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StageCommon/Stage.h>
#include <HighQueue/Producer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export QueueProducer : public Stage
        {
        public:
            QueueProducer();

            // Implement Stage
            virtual void handle(Message & message);

            // todo: virtual void configure(const ConfigurationPtr & configuration);
            void attachConnection(const ConnectionPtr & connection);

            virtual void validate();
        private:
            ConnectionPtr connection_;
            std::unique_ptr<Producer> producer_;
        };

    }
}