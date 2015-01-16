// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/ComponentBase.h>
#include <HighQueue/Producer.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        class MessageSource : public ComponentBase
        {
        public:
            MessageSource(ConnectionPtr & outConnection);

        protected:
            ConnectionPtr outConnection_;
            Producer producer_;
            Message outMessage_;
        };

        inline
        MessageSource::MessageSource(ConnectionPtr & outConnection)
            : ComponentBase()
            , outConnection_(outConnection)
            , producer_(outConnection_)
            , outMessage_(outConnection)
        {
        }
   }
}