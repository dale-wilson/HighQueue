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
            void dispatch(Message & message);

            virtual void handleHeartbeat(Message & message) = 0;
            virtual void handleShutdownMessage(Message & message) = 0;
            virtual void handleMessageType(Message::Meta::MessageType type, Message & message) = 0;
        };

        inline
        MessageDispatcher::~MessageDispatcher()
        {
        }

        inline
        void MessageDispatcher::dispatch(Message & message)
        {
            auto type = message.meta().type_;
            if(type == Message::Meta::Heartbeat)
            {
                LogTrace("Dispatch heartbeat");
                handleHeartbeat(message);
            }
            else if(type == Message::Meta::Shutdown)
            {
                LogTrace("Dispatch shutdown");
                handleShutdownMessage(message);
            }
            else
            { 
                handleMessageType(type, message);
            }
        }
   }
}