// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/Step.hpp>

#include <Common/Log.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export Stopper: public Step
        {
        public:
            explicit Stopper();

            // implement Step methods
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual std::ostream & usage(std::ostream & out) const;
            virtual void handle(Message & message);
            virtual void attachResources(const SharedResourcesPtr & resources);

        private:
            uint32_t messagesExpected_;
            uint32_t heartbeatsExpected_;
            uint32_t shutdownsExpected_;
            SharedResourcesPtr resources_;
            
            uint32_t messagesHandled_;
            uint32_t heartbeatsHandled_;
            uint32_t shutdownsHandled_;
        };
   }
}
