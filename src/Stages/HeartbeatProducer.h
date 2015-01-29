// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "HeartbeatProducerFwd.h"
#include <StagesSupport/AsioStageToMessage.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class HeartbeatProducer : public AsioStageToMessage
        {
        public:
            HeartbeatProducer(std::chrono::milliseconds interval = std::chrono::milliseconds(1000));

            virtual void start();
            virtual void stop();

        private:
            void startTimer();
            void handleTimer(const boost::system::error_code& error);
        private:
            typedef boost::asio::deadline_timer Timer;
            typedef boost::posix_time::millisec Interval;
            Interval interval_;
            std::unique_ptr<Timer> timer_;
            bool cancel_;
        };

        inline
        HeartbeatProducer::HeartbeatProducer(std::chrono::milliseconds interval)
            : interval_(interval.count())
            , cancel_(false)
        {
            setName("HeartbeatProducer"); // default name
        }

        inline
        void HeartbeatProducer::start()
        {
            timer_.reset(new Timer(*ioService_));
            startTimer();
        }

        inline
        void HeartbeatProducer::stop()
        {
            LogTrace("***HeartbeatProducer Cancel Timer");
            cancel_ = true;
            timer_->cancel();
            LogTrace("***HeartbeatProducer Stopping");
            AsioStageToMessage::stop();
        }

        inline
        void HeartbeatProducer::startTimer()
        {
            if(!stopping_)
            {
                timer_->expires_from_now(interval_);
                timer_->async_wait(boost::bind(
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
                outMessage_->setType(Message::Heartbeat);
                auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
                outMessage_->setTimestamp(timestamp);
                outMessage_->appendBinaryCopy(&timestamp, sizeof(timestamp));
                LogTrace("Publish Heartbeat: " << timestamp);
                send(*outMessage_);
            }
            startTimer();
        }
    }
}