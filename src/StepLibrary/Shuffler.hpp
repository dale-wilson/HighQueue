// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "ShufflerFwd.hpp"

#include <Steps/Step.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export Shuffler : public Step
        {
        public:
            static const size_t relativelyPrime_ = 101;
            Shuffler();

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(const SharedResourcesPtr & resources);
            virtual void attachResources(const SharedResourcesPtr & resources);
            virtual void validate();
            virtual void handle(Message & message);
            virtual void finish();
            virtual std::ostream & usage(std::ostream & out) const;
            virtual void logStats();

        private:
            void publishPendingMessages();
            void handleHeartbeat(Message & message);
            void handleShutdown(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            size_t prime_;
            typedef std::shared_ptr<Message> MessagePtr;
            typedef std::vector<MessagePtr> Messages;
            Messages pendingMessages_;
            uint64_t position_;

            size_t published_;
            size_t heartbeats_;
            size_t shutdowns_;
            size_t leftovers_;
        };

   }
}