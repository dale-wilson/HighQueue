// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "LeakerFwd.hpp"

#include <Steps/Step.hpp>

namespace HighQueue
{
    namespace Steps
    {
        class Steps_Export Leaker : public Step
        {
        public:
            static const size_t relativelyPrime_ = 101;
            Leaker();

            virtual std::ostream & usage(std::ostream & out) const override;
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration) override;
            virtual void validate() override;
            virtual void handle(Message & message) override;
            virtual void logStats() override;

        private:
            size_t count_;
            size_t every_;
            size_t offset_;
            bool leakHeartbeats_;
            bool leakShutdowns_;
            size_t messageNumber_;

            size_t published_;
            size_t leaked_;
        };

   }
}