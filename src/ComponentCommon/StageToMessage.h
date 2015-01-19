// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/Stage.h>
#include <HighQueue/Message.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class StageToMessage : public Stage
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

        StageToMessage::StageToMessage()
        {
        }

        StageToMessage::~StageToMessage()
        {
        }

        void StageToMessage::attachMemoryPool(const MemoryPoolPtr & pool)
        {
            outMessage_.reset(new Message(pool));
        }

        void StageToMessage::validate()
        {
            if(!outMessage_)
            {
                throw std::runtime_error("StageToMessage: Can't initialize output message. No memory pool attached.");
            }
        }        
   }
}