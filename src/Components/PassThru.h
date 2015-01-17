// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/MessageProcessor.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        template<typename CargoMessage>
        class PassThru : public MessageProcessor
        {
        public:
            enum CopyType
            {
                CopyBinary,
                CopyConstruct,
                CopyMove,
                CopyForward
            };
            static const char * copyTypeName(CopyType type)
            {
                switch(type)
                {
                case CopyBinary:
                    return "Binary";
                case CopyConstruct:
                    return "Construct";
                case CopyMove:
                    return "Move";
                case CopyForward:
                    return "Forward";
                default:
                    return "Invalid";
                }
            }

        public:
            PassThru(ConnectionPtr & inConnection, ConnectionPtr & outConnection, CopyType copyType, uint32_t messageCount = 0, bool quitOnEmptyMessage = true);
 
            virtual bool handleEmptyMessage(Message & message);
            virtual bool handleMessageType(Message::Meta::MessageType type, Message & message);
            virtual bool handleHeartbeat(Message & message);

        private:
            CopyType copyType_;
            uint32_t messageCount_;
            bool quitOnEmptyMessage_;
            uint32_t messagesHandled_;
        };

        template<typename CargoMessage>
        PassThru<CargoMessage>::PassThru(ConnectionPtr & inConnection, ConnectionPtr & outConnection, CopyType copyType, uint32_t messageCount, bool quitOnEmptyMessage)
            : MessageProcessor(inConnection, outConnection)
            , copyType_(copyType)
            , messageCount_(messageCount)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , messagesHandled_(0)
        {
        }

        template<typename CargoMessage>
        bool PassThru<CargoMessage>::handleEmptyMessage(Message & message)
        {
            outMessage_.meta() = message.meta();
            publish(outMessage_);
            return !quitOnEmptyMessage_;
        }

        template<typename CargoMessage>
        bool PassThru<CargoMessage>::handleMessageType(Message::Meta::MessageType type, Message & message)
        {
            switch(copyType_)
            {
                default:
                case CopyBinary:
                {
                    LogDebug("PassThru  copy binary.");
                    outMessage_.appendBinaryCopy(message.get(), message.getUsed());
                    outMessage_.meta() = message.meta();
                    publish(outMessage_);
                    return true;
                }
                case CopyConstruct:
                {
                    LogDebug("PassThru  copy construct.");
                    outMessage_.emplace<CargoMessage>(*message.get<CargoMessage>());
                    outMessage_.meta() = message.meta();
                    publish(outMessage_);
                    return true;
                }
                case CopyMove:
                {
                    LogDebug("PassThru  copy move.");
                    message.moveTo(outMessage_);
                    publish(outMessage_);
                    return true;
                }
                case CopyForward:
                {
                    LogDebug("PassThru  copy Forward.");
                    publish(message);
                    return true;
                }
            }
        }

        template<typename CargoMessage>
        bool PassThru<CargoMessage>::handleHeartbeat(Message & message)
        {
            outMessage_.appendBinaryCopy(message.get(), message.getUsed());
            outMessage_.meta() = message.meta();
            publish(outMessage_);
            return true;
            return true;
        }
   }
}