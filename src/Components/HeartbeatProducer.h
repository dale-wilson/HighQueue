// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "HeartbeatProducerFwd.h"
#include <ComponentCommon/MessageSource.h>
#include <ComponentCommon/AsioService.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Components
    {
        class HeartbeatProducer : public MessageSource
        {
        public:
            HeartbeatProducer(
                AsioServicePtr & ioService,
                ConnectionPtr & connection, 
                std::chrono::milliseconds interval);

            virtual void run();
            virtual void doStop();
        private:
            void startTimer();
            void handleTimer(const boost::system::error_code& error);
        private:
            AsioServicePtr ioService_;
            boost::posix_time::millisec interval_;
            boost::asio::deadline_timer timer_;
        };

        HeartbeatProducer::HeartbeatProducer(
            AsioServicePtr & ioService,
            ConnectionPtr & connection,
            std::chrono::milliseconds interval)
            : MessageSource(connection)
            , ioService_(ioService)
            , interval_(interval.count())
            , timer_(*ioService)
        {
            outMessage_.meta().type_ = Message::Meta::Heartbeat;
        }

        inline
        void HeartbeatProducer::run()
        {
            startTimer();
            while(!stopping_)
            {
                ioService_->run_one();
            }
        }

        inline
        void HeartbeatProducer::doStop()
        {
            MessageSource::stop();
            timer_.cancel();
        }

        inline
        void HeartbeatProducer::startTimer()
        {
            if(!stopping_)
            {
                timer_.expires_from_now(interval_);
                timer_.async_wait(boost::bind(
                    &HeartbeatProducer::handleTimer, this, boost::asio::placeholders::error));
            }
        }

        inline
        void HeartbeatProducer::handleTimer(const boost::system::error_code& error)
        {
            if(error)
            {
                if(!stopping_)
                {
                    // todo: write/find a real logger
                    LogError("Error in HeartbeatProducer " << error);
                }
            }
            else if(!paused_)
            {
                outMessage_.meta().timestamp_ = std::chrono::steady_clock::now().time_since_epoch().count();
                outMessage_.appendBinaryCopy(&outMessage_.meta().timestamp_, sizeof(outMessage_.meta().timestamp_));
                LogTrace("Publish Heartbeat: " << outMessage_.meta().timestamp_);
                publish(outMessage_);
            }
            startTimer();
        }
    }
}