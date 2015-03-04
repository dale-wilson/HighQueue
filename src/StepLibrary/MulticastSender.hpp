// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "MulticastSenderFwd.hpp"
#include <Steps/AsioStep.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class MulticastSender: public AsioStep
        {
        public:
            typedef boost::asio::ip::address Address; 
            typedef boost::asio::ip::udp::endpoint Endpoint;
            typedef boost::asio::ip::udp::socket Socket;
        public:
            MulticastSender();
            virtual ~MulticastSender();

            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration) override;
            virtual void configureResources(const SharedResourcesPtr & resources) override;

            virtual void validate() override;
            virtual void start() override;
            virtual void handle(Message & message) override;
            virtual void finish() override;
            virtual std::ostream & usage(std::ostream & out) const override;

            Socket  & socket();
        private:
            std::string multicastGroupIP_;
            std::string bindIP_;

            unsigned short portNumber_;
            Address multicastGroup_;
            Endpoint endpoint_;
            Endpoint bindpoint_;

            std::unique_ptr<Socket> socket_;
            uint32_t messageCount_;
            uint32_t errorCount_;
        };
   }
}
