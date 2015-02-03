// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "ShufflerFwd.h"

#include <Steps/StepToMessage.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export Shuffler : public StepToMessage
        {
        public:
            static const size_t relativelyPrime_ = 101;
            explicit Shuffler(size_t lookAhead = 50);

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(SharedResources & resources);
            virtual void attachResources(SharedResources & resources);
            virtual void validate();
            virtual void handle(Message & message);

        private:
            void publishPendingMessages();

            void handleHeartbeat(Message & message);
            void handleShutdown(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            std::vector<Message> pendingMessages_;
            uint64_t position_;
        };

   }
}