// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.h>
#include <HighQueue/Producer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export QueueProducer : public Step
        {
        public:
            QueueProducer();

            void configureSolo(bool solo);

            // Implement Step
            virtual void handle(Message & message);

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(BuildResources & resources);
            void attachConnection(const ConnectionPtr & connection);

            virtual void validate();
        private:
            bool solo_;
            ConnectionPtr connection_;
            std::unique_ptr<Producer> producer_;
        };
    }
}