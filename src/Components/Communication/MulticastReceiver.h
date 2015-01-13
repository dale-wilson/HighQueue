// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "MulticastReceiverFwd.h"
#include <Communication/AsioService.h>
#include <Communication/HeaderGenerator.h>
#include <HighQueue/Producer.h>

namespace HighQueue
{
    namespace Communication
    {
        struct MulticastConfiguration
        {
            std::string multicastGroupIP_;
            std::string listenInterfaceIP_;
            std::string bindIP_;
            unsigned short portNumber_;

            MulticastConfiguration(
                const std::string & multicastGroupIP,
                const std::string & listenInterfaceIP,
                const std::string & bindIP,
                unsigned short portNumber)
                : multicastGroupIP_(multicastGroupIP)
                , listenInterfaceIP_(listenInterfaceIP)
                , bindIP_(bindIP)
                , portNumber_(portNumber)
            {}
        };

        template<typename HeaderGenerator = NullHeaderGenerator>
        class MulticastReceiver: public std::enable_shared_from_this<MulticastReceiver<HeaderGenerator> >
        {
        public:
            MulticastReceiver(
                AsioServicePtr & ioService,
                ConnectionPtr & connection
                );
            ~MulticastReceiver();

            bool configure(const MulticastConfiguration & configuration);
            void start();
            void stop();
            void pause();
            void resume();

            boost::asio::ip::address listenInterface()const;
            unsigned short portNumber()const;
            boost::asio::ip::address multicastGroup()const;
            boost::asio::ip::address bindAddress()const;
            boost::asio::ip::udp::endpoint endpoint()const;
            boost::asio::ip::udp::endpoint senderEndpoint()const;
            boost::asio::ip::udp::socket & socket();
            bool joined()const;

        private:
            void MulticastReceiver::startRead();
            void handleReceive(const boost::system::error_code& error, size_t bytesReceived);

        private:
            HeaderGenerator headerGenerator_;
            
            AsioServicePtr ioService_;
            boost::asio::ip::udp::socket socket_;
            ConnectionPtr connection_;
            Producer producer_;
            Message message_;
            bool joined_;
            bool stopping_;

            struct MCastInfo
            {
                boost::asio::ip::address listenInterface_;
                unsigned short portNumber_;
                boost::asio::ip::address multicastGroup_;
                boost::asio::ip::address bindAddress_;
                boost::asio::ip::udp::endpoint endpoint_;
                boost::asio::ip::udp::endpoint senderEndpoint_;

                MCastInfo(const MulticastConfiguration & configuration)
                    : listenInterface_(boost::asio::ip::address::from_string(configuration.listenInterfaceIP_))
                    , portNumber_(configuration.portNumber_)
                    , multicastGroup_(boost::asio::ip::address::from_string(configuration.multicastGroupIP_))
                    , bindAddress_(boost::asio::ip::address::from_string(configuration.bindIP_))
                    , endpoint_(listenInterface_, configuration.portNumber_)
                {
                }
            };
            std::unique_ptr<MCastInfo> mcast_;
            std::shared_ptr<MulticastReceiver<HeaderGenerator> > me_;

        };

        template<typename HeaderGenerator>
        MulticastReceiver<HeaderGenerator>::MulticastReceiver(
            AsioServicePtr & ioService,
            ConnectionPtr & connection)
            : ioService_(ioService)
            , socket_(*ioService_)
            , connection_(connection)
            , producer_(connection)
            , message_(connection)
            , joined_(false)
            , stopping_(false)
        {
        }

        template<typename HeaderGenerator>
        MulticastReceiver<HeaderGenerator>::~MulticastReceiver()
        {
            stop();
        }

        template<typename HeaderGenerator>
        bool MulticastReceiver<HeaderGenerator>::configure(const MulticastConfiguration & configuration)
        {
            try
            {
                 mcast_.reset(new MCastInfo(configuration));
                socket_.open(mcast_->endpoint_.protocol());
                socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
                boost::asio::ip::udp::endpoint bindpoint(mcast_->bindAddress_, mcast_->portNumber_);
                socket_.bind(bindpoint);

                // Join the multicast group
                boost::asio::ip::multicast::join_group joinRequest(
                    mcast_->multicastGroup_.to_v4(),
                    mcast_->listenInterface_.to_v4());
                socket_.set_option(joinRequest);
            }
            catch(const std::exception & ex)
            {
                // todo logging
                std::cerr << "Failed to initialize Multicast group: " << ex.what() << std::endl;
                return false;
            }
            me_ = shared_from_this();
            joined_ = true;
            return true;
        }

        template<typename HeaderGenerator>
        void MulticastReceiver<HeaderGenerator>::start()
        {
            startRead();
        }

        template<typename HeaderGenerator>
        void MulticastReceiver<HeaderGenerator>::startRead()
        {
            if(!stopping)
            {
                messageHeaderGenerator_.addHeader(message_);
                socket_.async_receive_from(
                    boost::asio::buffer(message_.getWritePosition(), message_.available()),
                    senderEndpoint_,
                    boost::bind(&MulticastReceiver::handleReceive,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)
                    );
            }
        }

        template<typename HeaderGenerator>
        void MulticastReceiver<HeaderGenerator>::handleReceive(
            const boost::system::error_code& error,
            size_t bytesReceived)
        {
            if(error)
            {
                // todo: write a real logger
                std::cerr << "Error in multicast reader.  Shutting down. Message: " << error << std::endl;
            }
            else
            {
                message_.addUsed(bytesReceived);
                producer_.publish(message_);
                startRead();
            }
        }

        template<typename HeaderGenerator>
        void MulticastReceiver<HeaderGenerator>::stop()
        {
            stopping_ = true;
            try
            {
                pause();
                // attempt to cancel any receive requests in progress.
                socket_.cancel();
            }
            catch(...)
            {
            }
            me_.reset();
        }

        template<typename HeaderGenerator>
        void MulticastReceiver<HeaderGenerator>::pause()
        {
            // Temporarily leave the group
            if(joined_)
            {
                boost::asio::ip::multicast::leave_group leaveRequest(
                    mcast_->multicastGroup_.to_v4(),
                    mcast_->listenInterface_.to_v4());
                socket_.set_option(leaveRequest);
                joined_ = false;
            }
        }

        template<typename HeaderGenerator>
        void MulticastReceiver<HeaderGenerator>::resume()
        {
            if(!joined_)
            {
                // rejoin the multicast group
                boost::asio::ip::multicast::join_group joinRequest(
                    mcast_->multicastGroup_.to_v4(),
                    mcast_->listenInterface_.to_v4());
                socket_.set_option(joinRequest);
                joined_ = true;
            }
        }

        template<typename HeaderGenerator>
        boost::asio::ip::address MulticastReceiver<HeaderGenerator>::listenInterface()const
        {
            return listenInterface_;
        }

        template<typename HeaderGenerator>
        unsigned short MulticastReceiver<HeaderGenerator>::portNumber()const
        {
            return portNumber_;
        }

        template<typename HeaderGenerator>
        boost::asio::ip::address MulticastReceiver<HeaderGenerator>::multicastGroup()const
        {
            return multicastGroup_;
        }

        template<typename HeaderGenerator>
        boost::asio::ip::address MulticastReceiver<HeaderGenerator>::bindAddress()const
        {
            return bindAddress_;
        }

        template<typename HeaderGenerator>
        boost::asio::ip::udp::endpoint MulticastReceiver<HeaderGenerator>::endpoint()const
        {
            return endpoint_;
        }

        template<typename HeaderGenerator>
        boost::asio::ip::udp::endpoint MulticastReceiver<HeaderGenerator>::senderEndpoint()const
        {
            return senderEndpoint_;
        }

        template<typename HeaderGenerator>
        boost::asio::ip::udp::socket & MulticastReceiver<HeaderGenerator>::socket()
        {
            return socket_;
        }

        template<typename HeaderGenerator>
        bool MulticastReceiver<HeaderGenerator>::joined()const
        {
            return joined_;
        }
   }
}