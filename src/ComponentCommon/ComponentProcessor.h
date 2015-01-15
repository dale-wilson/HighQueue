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
        class ComponentProcessor : public ComponentBase
        {
        public:
            ComponentProcessor(ConnectionPtr & inConnection, ConnectionPtr & outConnection);

        protected:
            virtual void handleEmptyMessage(Message & message) = 0;
            virtual void handleMessageType(Message::Meta::MessageType type, Message & message) = 0;
            virtual void handleHeartbeat(Message & message) = 0;
            virtual void handleDataMessage(Message & message) = 0;

        private:
            ConnectionPtr inConnection_;
            Consumer consumer_;
            Message inMessage_;

            ConnectionPtr outConnection_;
            Producer producer_;
            Message outMessage_;
        };

        
        ComponentProcessor::ComponentProcessor(ConnectionPtr & inConnection)
            : ComponentBase()
            , inConnection_(inConnection)
            , consumer_(inConnection_)
            , inMessage_(inConnection)
            , outConnection_(outConnection)
            , producer_(outConnection_)
            , outMessage_(outConnection)
        {
        }
   }
}