// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/StageToMessage.h>
#include <StagesSupport/AsioService.h>

//#include <HighQueue/Message.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class  Stages_Export AsioStageToMessage : public StageToMessage
        {

        public:
            AsioStageToMessage();
            virtual ~AsioStageToMessage();

            virtual void attachResources(BuildResources & resources);
            virtual void validate();

            virtual void start();
            virtual void finish();
        protected:
            AsioServicePtr ioService_;
            std::shared_ptr<Stage> me_;
        };
   }
}