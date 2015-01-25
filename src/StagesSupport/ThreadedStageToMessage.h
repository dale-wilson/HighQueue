// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/StageToMessage.h>
#include <HighQueue/Message.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class  Stages_Export ThreadedStageToMessage : public StageToMessage
        {

        public:
            ThreadedStageToMessage();
            virtual ~ThreadedStageToMessage();
            virtual void start();
            virtual void finish();
            virtual void run() = 0;
        protected:
            void startThread();
        protected:
            std::shared_ptr<Stage> me_;
            std::thread thread_;

        };
   }
}