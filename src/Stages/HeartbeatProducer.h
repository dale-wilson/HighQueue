// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include "HeartbeatProducerFwd.h"
#include <StageCommon/ThreadedStageToMessage.h>
#include <StageCommon/AsioService.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class HeartbeatProducer : public ThreadedStageToMessage
        {
        public:
            HeartbeatProducer(std::chrono::milliseconds interval);

            virtual void attachIoService(const AsioServicePtr & ioService);
            virtual void run();
            virtual void stop();

            virtual void handle(Message & message);

        private:
            void startTimer();
            void handleTimer(const boost::system::error_code& error);
        private:
            AsioServicePtr ioService_;
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
        void HeartbeatProducer::handle(Message & message)
        {
            throw std::runtime_error("HeartbeatProducer does not accept incoming Messages");
        }

        inline
        void HeartbeatProducer::attachIoService(const AsioServicePtr & ioService)
        {
            ioService_ = ioService;
            timer_.reset(new Timer(*ioService));
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
        void HeartbeatProducer::stop()
        {
            LogTrace("***HeartbeatProducer Stopping");
            ThreadedStageToMessage::stop();
            LogTrace("***HeartbeatProducer Cancel Timer");
            cancel_ = true;
            timer_->cancel();
            LogTrace("***HeartbeatProducer stop exits");
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