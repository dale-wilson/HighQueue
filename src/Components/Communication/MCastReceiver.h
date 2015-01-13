// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Communication/Communication_Export.h>
#include <Communication/AsioServiceFwd.h>
namespace HighQueue
{
    namespace Communication
    {
        class Communication_Export MCastReceiver
        {
        public:
            MCastReceiver(
                AsioServicePtr ioService,
                const std::string & multicastGroupIP,
                const std::string & listenInterfaceIP,
                const std::string & bindIP,
                unsigned short portNumber
                );
            ~MCastReceiver();

        private:
            AsioServicePtr ioService_;
            boost::asio::ip::address listenInterface_;
            unsigned short portNumber_;
            boost::asio::ip::address multicastGroup_;
            boost::asio::ip::address bindAddress_;
            boost::asio::ip::udp::endpoint endpoint_;
            boost::asio::ip::udp::endpoint senderEndpoint_;
            boost::asio::ip::udp::socket socket_;

        };
    }
}