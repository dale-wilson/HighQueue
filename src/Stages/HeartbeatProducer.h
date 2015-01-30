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
        class Stages_Export HeartbeatProducer : public AsioStageToMessage
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
    }
}