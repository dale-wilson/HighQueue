// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/Stage.h>
#include <HighQueue/MessageFwd.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class  Stages_Export StageToMessage : public Stage
        {

        public:
            StageToMessage();
            virtual ~StageToMessage();

            // implement stage methods
            virtual void attachMemoryPool(const MemoryPoolPtr & pool);
            virtual void validate();
 
        protected:
            std::unique_ptr<Message> outMessage_;
        };

   }
}