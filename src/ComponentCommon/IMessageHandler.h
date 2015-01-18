// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/MessageFwd.h>

namespace HighQueue
{
    namespace Components
    {
        /// @brief An interface used to pass messages to something that handles messages.
        class IMessageHandler
        {
        public:
            virtual ~IMessageHandler() {}
            /// @brief Implement this and voila you're a message handler
            /// @param message is the one to handle.
            /// @returns false if we should stop now.
            virtual void handleMessage(Message & message) = 0;
        };
        typedef std::shared_ptr<IMessageHandler> IMessageHandlerPtr;
   }
}