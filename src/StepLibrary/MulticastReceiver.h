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
            typedef boost::asio::ip::address Address;
            typedef boost::asio::ip::udp::endpoint Endpoint;
            typedef boost::asio::ip::udp::socket Socket;
            
        public:
            MulticastReceiver();
            virtual ~MulticastReceiver();

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);
            virtual void configureResources(SharedResources & resources);

            virtual void validate();
            virtual void start();

            virtual void pause();
            virtual void resume();
            virtual void stop();

            Address listenInterface()const;
            unsigned short portNumber()const;
            Address multicastGroup()const;
            Address bindAddress()const;
            Endpoint endpoint()const;
            Endpoint senderEndpoint()const;
            Socket & socket();
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
            std::unique_ptr<Socket> socket_;

            struct MCastInfo
            {
                Address multicastGroup_;
                uint16_t portNumber_;
                Address listenInterface_;
                Address bindAddress_;
                Endpoint endpoint_;
                Endpoint senderEndpoint_;

                MCastInfo(
                    const std::string & multicastGroupIP,
                    uint16_t portNumber,
                    const std::string & listenInterfaceIP,
                    const std::string & bindIP

                    )
                    : multicastGroup_(Address::from_string(multicastGroupIP))
                    , portNumber_(portNumber)
                    , listenInterface_(Address::from_string(listenInterfaceIP))
                    , bindAddress_(Address::from_string(bindIP))
                    , endpoint_(listenInterface_, portNumber_)
                {
                }
            };
            std::unique_ptr<MCastInfo> mcast_;
        };

   }
}
