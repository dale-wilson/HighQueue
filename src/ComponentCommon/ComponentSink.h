// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/ComponentBase.h>
#include <HighQueue/Consumer.h>

#include <ComponentCommon/DebugMessage.h>

////////////////////////////////////////
#pragma message("WARNING UNDER CONSTRUCTION.  NRFPT")

namespace HighQueue
{
    namespace Components
    {
        class ComponentSink : public ComponentBase
        {
        public:
            ComponentSink(ConnectionPtr & inConnection);

        protected:
            virtual void handleEmptyMessage(Message & message) = 0;
            virtual void handleMessageType(Message::Meta::MessageType type, Message & message) = 0;
            virtual void handleHeartbeat(Message & message) = 0;
            virtual void handleDataMessage(Message & message) = 0;

        private:
            ConnectionPtr inConnection_;
            Consumer consumer_;
            Message inMessage_;
        };

        
        ComponentSink::ComponentSink(ConnectionPtr & inConnection)
            : ComponentBase()
            , inConnection_(inConnection)
            , consumer_(inConnection_)
            , inMessage_(inConnection)
        {
        }
   }
}