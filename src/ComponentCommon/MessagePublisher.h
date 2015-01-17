// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <ComponentCommon/IMessageHandler.h>
#include <ComponentCommon/MessagePublisher.h>
#include <HighQueue/Producer.h>

namespace HighQueue
{
    namespace Components
    {
        /// @brief An interface used to pass messages to something that handles messages.
        class MessagePublisher : public IMessageHandler
        {
        public:
            explicit MessagePublisher(ConnectionPtr & connection);
            /// @brief Implement IMessageHandler
            virtual bool handleMessage(Message & message);

        public:
            Producer producer_;
        };


        inline
        MessagePublisher::MessagePublisher(ConnectionPtr & connection)
        : producer_(connection)
        {
        }

        inline
        bool MessagePublisher::handleMessage(Message & message)
        {
            producer_.publish(message);
            return true;
        }
    }
}