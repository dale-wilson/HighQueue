// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <HighQueue/Message.h>
namespace HighQueue
{
    namespace Communication
    {
        class NullHeaderGenerator
        {
        public:
            void addHeader(Message & message)
            {
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
        private:
            byte_t value_[2];
        };

        template<typename Header, typename... ArgTypes>
        class ObjectHeaderGenerator
        {
        public:
            void addHeader(Message & message)
            {
                message.emplace<Header>(ArgTypes&&... args);
            }
        };

    }
}