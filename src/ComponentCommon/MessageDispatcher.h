// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <HighQueue/Message.h>

namespace HighQueue
{
    namespace Components
    {
        class MessageDispatcher
        {
        public:
            virtual ~MessageDispatcher();
            bool dispatch(Message & message);

            virtual bool handleEmptyMessage(Message & message) = 0;
            virtual bool handleMessageType(Message::Meta::MessageType type, Message & message) = 0;
            virtual bool handleHeartbeat(Message & message) = 0;
        };

        inline
        MessageDispatcher::~MessageDispatcher()
        {
        }

        inline
        bool MessageDispatcher::dispatch(Message & message)
        {
            if(message.isEmpty())
            {
                return handleEmptyMessage(message);
            }
            auto type = message.meta().type_;
            if(type == Message::Meta::Heartbeat)
            {
                return handleHeartbeat(message);
            }
            return handleMessageType(type, message);
        }

   }
}