// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "ShufflerFwd.h"

#include <StagesSupport/StageToMessage.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export Shuffler : public StageToMessage
        {
        public:
            static const size_t relativelyPrime_ = 101;
            Shuffler(size_t lookAhead = 50);

            virtual void attachConnection(const ConnectionPtr & connection);
            virtual void attachMemoryPool(const MemoryPoolPtr & memoryPool);

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