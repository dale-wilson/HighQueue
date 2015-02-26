// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.hpp>
#include <HighQueue/Producer.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export SendToQueue : public Step
        {
        public:
            SendToQueue();

            // Implement Step
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void attachResources(const SharedResourcesPtr & resources);
            virtual void validate();
            virtual void start();
            virtual void handle(Message & message);
            virtual void stop();
            virtual std::ostream & usage(std::ostream & out) const;

        private:
            std::string queueName_;
            ConnectionPtr connection_;
            std::unique_ptr<Producer> producer_;
        };
    }
}