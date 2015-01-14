// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <HighQueue/Message.h>
namespace HighQueue
{
    namespace Components
    {
        class NullHeaderGenerator
        {
        public:
            void addHeader(Message & message)
            {
            }
            bool consumeHeader(Message & message)
            {
                return true;
            }
        };

        template<byte_t MsgType, byte_t Version>
        class TypeVersionHeaderGenerator
        {
        public:
            TypeVersionHeaderGenerator()
            {
                value_[0] = MsgType;
                value_[1] = Version;
            }
            void addHeader(Message & message)
            {
                message.appendBinaryCopy(value_, sizeof(value_));
            }
            bool consumeHeader(Message & message)
            {
                auto ptr message.getReadPointer<byte_t>();
                if(message.needUnread(sizeof(value_)) && ptr[0] == value_[0] && ptr[1] == value_[1])
                {
                    message.addRead(sizeof(value_));
                    return true;
                }
                return false;
            }
        private:
            byte_t value_[2];
        };

        template<typename Header, typename... ArgTypes>
        class ObjectHeaderGenerator
        {
        public:
            void addHeader(Message & message)
            {
                message.appendEmplace<Header>(ArgTypes&&... args);
            }
            bool consumeHeader(Message & message)
            {
                if(message.needAvailable(sizeof(Header)))
                {
                    message.addRead(sizeof(Header));
                    return true;
                }
                return false;
            }
        };

    }
}