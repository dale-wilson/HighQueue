// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <Steps/StepToMessage.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class  Steps_Export ThreadedStepToMessage : public StepToMessage
        {

        public:
            ThreadedStepToMessage();
            virtual ~ThreadedStepToMessage();
            virtual void start() override;
            virtual void finish() override;
            virtual void run() = 0;
        protected:
            void startThread();
        protected:
            std::shared_ptr<Step> me_;
            std::thread thread_;

        };
   }
}