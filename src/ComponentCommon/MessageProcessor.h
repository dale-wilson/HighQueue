// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/ComponentBase.h>
#include <ComponentCommon/MessageDispatcher.h>

#include <HighQueue/Consumer.h>
#include <HighQueue/Producer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class MessageProcessor: public ComponentBase, public MessageDispatcher
        {
        public:
            MessageProcessor(ConnectionPtr & inConnection, ConnectionPtr & outConnection);

            virtual void run();
            virtual bool handleMessage(Message & message);

            void attachHandler(IMessageHandlerPtr & handler)
            {
                handler_ = handler;
            }

            bool publish(Message & message);

        protected:
            ConnectionPtr inConnection_;
            Consumer consumer_;
            Message inMessage_;

            ConnectionPtr outConnection_;
            IMessageHandlerPtr handler_;
            Message outMessage_;
        };

        inline
            MessageProcessor::MessageProcessor(ConnectionPtr & inConnection, ConnectionPtr & outConnection)
            : ComponentBase()
            , inConnection_(inConnection)
            , consumer_(inConnection_)
            , inMessage_(inConnection)
            , outConnection_(outConnection)
            , handler_(new MessagePublisher(outConnection_))
            , outMessage_(outConnection)
        {
        }

        inline
        void MessageProcessor::run()
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
        bool MessageProcessor::handleMessage(Message & message)
        {
            return dispatch(message);
        }

        inline
        bool MessageProcessor::publish(Message & message)
        {
            return handler_->handleMessage(message);
        }

   }
}