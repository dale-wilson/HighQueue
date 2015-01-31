// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#ifdef DISABLED
#include "MulticastReceiverFwd.h"
#include <Steps/AsioService.h>
#include <Steps/ThreadedStepToMessage.h>

namespace HighQueue
{
    namespace Steps
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
        private:
            friend class MulticastReceiver;
            MulticastConfiguration()
            : portNumber_(0)
            {}
        };

        class MulticastReceiver: public ThreadedStepToMessage
        {
        public:
            MulticastReceiver();

            bool configure(const MulticastConfiguration & configuration);
            virtual bool configure(ConfigurationNodePtr & config);
            virtual void attachIoService(const AsioServicePtr & ioService);
            virtual void start();
            virtual void run();
            virtual void stop();
            virtual void pause();
            virtual void resume();

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

            AsioServicePtr ioService_;
            typedef boost::asio::ip::udp::socket Socket;
            std::unique_ptr<Socket> socket_;
            MulticastConfiguration configuration_;

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
        };

        inline
        MulticastReceiver::MulticastReceiver()
            : joined_(false)
        {
            setName("MulicastReceiver"); // default name
        }

        inline
        void MulticastReceiver::attachIoService(const AsioServicePtr & ioService)
        {
            ioService_ = ioService;
            socket_.reset(new Socket(*ioService));
        }


        inline
        bool MulticastReceiver::configure(const MulticastConfiguration & configuration)
        {
            configuration_ = configuration;
        }
        inline
        bool MulticastReceiver::configure(ConfigurationNodePtr & config)
        {
            int todo;
        }


        inline
        void MulticastReceiver::start()
        {
            mcast_.reset(new MCastInfo(configuration_));
            socket_->open(mcast_->endpoint_.protocol());
            socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
            boost::asio::ip::udp::endpoint bindpoint(mcast_->bindAddress_, mcast_->portNumber_);
            socket_->bind(bindpoint);

            // Join the multicast group
            boost::asio::ip::multicast::join_group joinRequest(
                mcast_->multicastGroup_.to_v4(),
                mcast_->listenInterface_.to_v4());
            socket_->set_option(joinRequest);
            joined_ = true;
            ThreadedStepToMessage::start();       
            //catch(const std::exception & ex)
            //{
            //    LogError("Failed to initialize Multicast group: " << ex.what());
            //}
        }

        inline
        void MulticastReceiver::run()
        {
            startRead();
            while(!stopping_)
            {
                ioService_->run_one();
            }
        }

        inline
        void MulticastReceiver::startRead()
        {
            if(!stopping_)
            {
                socket_->async_receive_from(
                    boost::asio::buffer(outMessage_->getWritePosition(), outMessage_->available()),
                    mcast_->senderEndpoint_,
                    boost::bind(&MulticastReceiver::handleReceive,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)
                    );
            }
        }

        inline
        void MulticastReceiver::handleReceive(
            const boost::system::error_code& error,
            size_t bytesReceived)
        {
            if(error)
            {
                LogError("Error in multicast reader.  Shutting down. Message: " << error);
                stop();
            }
            else
            {
                outMessage_->setType(Message::MulticastPacket);
                auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
                outMessage_->setTimestamp(timestamp);
                outMessage_->addUsed(bytesReceived);
                send(*outMessage_);
                startRead();
            }
        }

        inline
        void MulticastReceiver::pause()
        {
            // Temporarily leave the group
            if(joined_)
            {
                boost::asio::ip::multicast::leave_group leaveRequest(
                    mcast_->multicastGroup_.to_v4(),
                    mcast_->listenInterface_.to_v4());
                socket_->set_option(leaveRequest);
                joined_ = false;
            }
            ThreadedStepToMessage::pause();
        }

        inline
        void MulticastReceiver::resume()
        {
            ThreadedStepToMessage::resume();
            if(!joined_)
            {
                // rejoin the multicast group
                boost::asio::ip::multicast::join_group joinRequest(
                    mcast_->multicastGroup_.to_v4(),
                    mcast_->listenInterface_.to_v4());
                socket_->set_option(joinRequest);
                joined_ = true;
            }
        }

        inline
        boost::asio::ip::address MulticastReceiver::listenInterface()const
        {
            return mcast_->listenInterface_;
        }

        inline
        unsigned short MulticastReceiver::portNumber()const
        {
            return mcast_->portNumber_;
        }

        inline
        boost::asio::ip::address MulticastReceiver::multicastGroup()const
        {
            return mcast_->multicastGroup_;
        }

        inline
        boost::asio::ip::address MulticastReceiver::bindAddress()const
        {
            return mcast_->bindAddress_;
        }

        inline
        boost::asio::ip::udp::endpoint MulticastReceiver::endpoint()const
        {
            return mcast_->endpoint_;
        }

        inline
        boost::asio::ip::udp::endpoint MulticastReceiver::senderEndpoint()const
        {
            return mcast_->senderEndpoint_;
        }

        inline
        boost::asio::ip::udp::socket & MulticastReceiver::socket()
        {
            return *socket_;
        }

        inline
        bool MulticastReceiver::joined()const
        {
            return joined_;
        }
   }
}
#endif //DISABLED
