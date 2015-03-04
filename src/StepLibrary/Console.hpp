// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/ThreadedStepToMessage.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export  Console : public ThreadedStepToMessage
        {
        public:
            Console();
            virtual ~Console();

            virtual std::ostream & usage(std::ostream & out) const override;
            virtual bool configureParameter(const std::string & name,  const ConfigurationNode & config) override;
            virtual void attachResources(const SharedResourcesPtr & resources) override;
            virtual void run() override;
        protected:
            SharedResourcesPtr resources_;
            std::string prompt_;
        };

   }
}