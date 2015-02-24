// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "OrderedMergeFwd.h"
#include <StepLibrary/GapMesssage.h>
#include <Steps/StepToMessage.h>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export OrderedMerge : public StepToMessage
        {
        public:
            OrderedMerge();

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(SharedResources & resources);

            virtual void attachResources(SharedResources & resources);
            virtual void validate();

            virtual void handle(Message & message);

            virtual void finish();
            virtual std::ostream & usage(std::ostream & out) const;

            virtual void publishStats();

        private:
            bool findAndPublishGap();
            void publishGapMessage(uint32_t gapStart, uint32_t gapEnd);
            void publishPendingMessages();

            void handleHeartbeat(Message & message);
            void handleShutdown(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            size_t maxDelayHeartbeats_;
            size_t heartbeatDelays_;
            uint32_t expectedSequenceNumber_;
            uint32_t highestStashed_;
            typedef std::shared_ptr<Message> MessagePtr;
            typedef std::vector<MessagePtr> Messages;
            Messages pendingMessages_;
            uint32_t lastHeartbeatSequenceNumber_;
            uint32_t lastHeartbeatHighestStashed_;

            size_t statReceived_;
            size_t statHeartbeats_;
            size_t statShutdowns_;
            size_t statData_;
            size_t statHeartbeatWithoutPublish_;
            size_t statShutdownPublishedGap_;
            size_t statArrivedInOrder_;
            size_t statPrevious_;
            size_t statStashed_;
            size_t statDuplicatesStash_;
            size_t statFuture_;
        };

   }
}
