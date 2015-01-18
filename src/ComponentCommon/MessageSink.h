// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/ComponentBase.h>
#include <ComponentCommon/MessageDispatcher.h>
#include <HighQueue/Consumer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class MessageSink: public ComponentBase, public MessageDispatcher
        {
        public:
            MessageSink(ConnectionPtr & inConnection);

            virtual void run();

            // Implement IMessageHandler
            virtual void handleMessage(Message & message);

            // provide default implementation of MessageDispatcher method
            virtual void handleShutdownMessage(Message & message);
            virtual void handleHeartbeat(Message & message);

        protected:
            ConnectionPtr inConnection_;
            Consumer consumer_;
            Message inMessage_;
        };

        inline
        MessageSink::MessageSink(ConnectionPtr & inConnection)
            : ComponentBase()
            , inConnection_(inConnection)
            , consumer_(inConnection_)
            , inMessage_(inConnection)
        {
        }

        inline
        void MessageSink::run()
        {
            LogTrace("MessageSink::run " << (void *) this << ": " << stopping_);
            while(!stopping_)
            {
                stopping_ = !consumer_.getNext(inMessage_);
                if(!stopping_)
                {
                    dispatch(inMessage_);
                }
                else
                {
                    LogTrace("MessageSink::stopped by getnext? " << (void *) this << ": " << stopping_);
                }
            }
            LogTrace("MessageSink::run returns " << (void *) this << ": " << stopping_);
        }

        inline
        void MessageSink::handleMessage(Message & message)
        {
            dispatch(message);
        }

        inline
        void MessageSink::handleShutdownMessage(Message & message)
        {
            LogTrace("MessageSink received shutdown messsage.");
            stop();
        }

        inline
            void MessageSink::handleHeartbeat(Message & message)
        {
            ; // what heartbeat?
        }

    }
}