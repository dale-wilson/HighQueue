// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "MulticastReceiverFwd.h"
#include <Steps/AsioStepToMessage.h>

namespace HighQueue
{
    namespace Steps
    {

        class MulticastReceiver: public AsioStepToMessage
        {
        public:
            MulticastReceiver();
            virtual ~MulticastReceiver();

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(BuildResources & resources);

            virtual void validate();
            virtual void start();

            virtual void pause();
            virtual void resume();
            virtual void stop();

            boost::asio::ip::address listenInterface()const;
            unsigned short portNumber()const;
            boost::asio::ip::address multicastGroup()const;
            boost::asio::ip::address bindAddress()const;
            boost::asio::ip::udp::endpoint endpoint()const;
            boost::asio::ip::udp::endpoint senderEndpoint()const;
            boost::asio::ip::udp::socket & socket();
            bool joined()const;

        private:
            void startRead();
            void handleReceive(const boost::system::error_code& error, size_t bytesReceived);

        private:
            
            bool joined_;
            bool canceled_;

            // setable parameters
            size_t packetSize_;
            std::string multicastGroupIP_;
            std::string listenInterfaceIP_;
            std::string bindIP_;
            unsigned short portNumber_;


            AsioServicePtr ioService_;
            typedef boost::asio::ip::udp::socket Socket;
            std::unique_ptr<Socket> socket_;

            struct MCastInfo
            {
                boost::asio::ip::address multicastGroup_;
                uint16_t portNumber_;
                boost::asio::ip::address listenInterface_;
                boost::asio::ip::address bindAddress_;
                boost::asio::ip::udp::endpoint endpoint_;
                boost::asio::ip::udp::endpoint senderEndpoint_;

                MCastInfo(
                    const std::string & multicastGroupIP,
                    uint16_t portNumber,
                    const std::string & listenInterfaceIP,
                    const std::string & bindIP

                    )
                    : multicastGroup_(boost::asio::ip::address::from_string(multicastGroupIP))
                    , portNumber_(portNumber)
                    , listenInterface_(boost::asio::ip::address::from_string(listenInterfaceIP))
                    , bindAddress_(boost::asio::ip::address::from_string(bindIP))
                    , endpoint_(listenInterface_, portNumber_)
                {
                }
            };
            std::unique_ptr<MCastInfo> mcast_;
        };

   }
}
