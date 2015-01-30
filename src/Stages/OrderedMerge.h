// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "OrderedMergeFwd.h"

#include <StagesSupport/StageToMessage.h>

namespace HighQueue
{
    namespace Stages
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

        class Stages_Export OrderedMerge : public StageToMessage
        {
        public:
            OrderedMerge();

            virtual bool configure(const ConfigurationNodePtr & configuration, BuildResources & resources);

            virtual void attach(BuildResources & resources);
            //virtual void attachConnection(const ConnectionPtr & connection);
            //virtual void attachMemoryPool(const MemoryPoolPtr & memoryPool);

            virtual void handle(Message & message);

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