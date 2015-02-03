// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "OrderedMergeFwd.h"

#include <Steps/StepToMessage.h>

namespace HighQueue
{
    namespace Steps
    {
        class GapMessage
        {
        public:
            GapMessage(uint32_t startGap, uint32_t gapEnd = 0)
                : startGap_(startGap)
                , endGap_(gapEnd)
            {
            }
            uint32_t & startGap()
            {
                return startGap_;
            }
            uint32_t & gapEnd()
            {
                return endGap_;
            }
        private:
            uint32_t startGap_;
            uint32_t endGap_;
        };

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

            std::ostream & writeStats(std::ostream & out);

        private:
            bool findAndPublishGap();
            void publishGapMessage(uint32_t gapStart, uint32_t gapEnd);
            void publishPendingMessages();

            void handleHeartbeat(Message & message);
            void handleShutdown(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            size_t expectedShutdowns_;
            size_t actualShutdowns_;
            uint32_t expectedSequenceNumber_;
            std::vector<Message> pendingMessages_;
            uint32_t lastHeartbeatSequenceNumber_;

            size_t statReceived_;
            size_t statHeartbeats_;
            size_t statShutdowns_;
            size_t statData_;
            size_t statHeartbeatWithoutPublish_;
            size_t statShutdownPublishedGap_;
            size_t statArrivedInOrder_;
            size_t statDuplicatesPrevious_;
            size_t statStashed_;
            size_t statDuplicatesStash_;
            size_t statFuture_;
        };

   }
}
