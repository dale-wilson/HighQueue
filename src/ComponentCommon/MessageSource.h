// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/ComponentBase.h>
#include <ComponentCommon/IMessageHandler.h>
#include <ComponentCommon/MessagePublisher.h>
#include <HighQueue/Producer.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class MessageSource : public ComponentBase
        {
        public:
            MessageSource(ConnectionPtr & outConnection);

            virtual bool handleMessage(Message & message);

            void attachHandler(IMessageHandlerPtr & handler)
            {
                handler_ = handler;
            }

            bool publish(Message & message);

        protected:
            ConnectionPtr outConnection_;
            IMessageHandlerPtr handler_;
            Message outMessage_;
        };

        inline
        MessageSource::MessageSource(ConnectionPtr & outConnection)
            : ComponentBase()
            , outConnection_(outConnection)
            , handler_(new MessagePublisher(outConnection_)) // todo delay this till initialization
            , outMessage_(outConnection)
        {
        }

        inline
        bool MessageSource::publish(Message & message)
        {
            return handler_->handleMessage(message);
        }

        inline
        bool MessageSource::handleMessage(Message & message)
        {
            throw std::runtime_error("This component does not handle messages, it generates them.");
        }
    }
}