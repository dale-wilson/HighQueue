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
        class MessageSink: public ComponentBase,public MessageDispatcher
        {
        public:
            MessageSink(ConnectionPtr & inConnection);

            virtual void run();

            // Implement IMessageHandler
            virtual bool handleMessage(Message & message);

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
            while(!stopping_)
            {
                stopping_ = !consumer_.getNext(inMessage_);
                if(!stopping_)
                {
                    stopping_ = !dispatch(inMessage_);
                }
            }
        }

        inline
        bool MessageSink::handleMessage(Message & message)
        {
            return dispatch(message);
        }

    }
}