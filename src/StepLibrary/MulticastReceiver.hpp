// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "MulticastReceiverFwd.hpp"
#include <Steps/AsioStepToMessage.hpp>

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
            virtual std::ostream & usage(std::ostream & out) const;

            Endpoint senderEndpoint()const;
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

            std::unique_ptr<Socket> socket_;

            Address multicastGroup_;
            Address listenInterface_;
            Address bindAddress_;
            Endpoint listenEndpoint_;
            Endpoint senderEndpoint_;
            Endpoint bindpoint_;

            uint32_t messagesReceived_;

        };

   }
}
