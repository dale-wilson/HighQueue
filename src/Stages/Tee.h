// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <StagesSupport/Stage.h>
#include <StagesSupport/Stage_Export.h>

#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        class Stages_Export Tee: public Stage
        {
        public:
            static const std::string keyOutput;
            /// @brief hard code the line width.
            static const size_t bytesPerLine = 16;

            /// @brief construct
            Tee();

            virtual bool configure(const ConfigurationNodePtr & config);
            virtual void start();

            /// @brief Attach an ostream.  If none, don't dump the data.
            void attachOutputStream(std::ostream * outputStream);

            // implement stage methods
            virtual void handle(Message & message);

        private:
            void hexDump(byte_t * message, size_t size);
        private:
            std::string outputName_;
            std::ostream * out_;
            std::ofstream outfile_;
        };

   }
}