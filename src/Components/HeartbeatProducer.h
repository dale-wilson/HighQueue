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
            bool cancel_;
        };

        HeartbeatProducer::HeartbeatProducer(
            AsioServicePtr & ioService,
            ConnectionPtr & connection,
            std::chrono::milliseconds interval)
            : MessageSource(connection)
            , ioService_(ioService)
            , interval_(interval.count())
            , timer_(*ioService)
            , cancel_(false)
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
            LogTrace("***HeartbeatProducer Cancel Timer");
            cancel_ = true;
            timer_.cancel();
            LogTrace("***HeartbeatProducer Stopping");
            MessageSource::doStop();
            LogTrace("***HeartbeatProducer Exiting doStop");
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
                LogTrace("HeartbeatProducer: timer canceled.");
                if(!cancel_) // did we expect this?
                {
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