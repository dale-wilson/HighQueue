// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "StageFwd.h"
#include <StageCommon/Stage_Export.h>
#include <StageCommon/AsioServiceFwd.h>
#include <StageCommon/Configuration.h>

#include "HighQueue/Message.h"
#include "HighQueue/ConnectionFwd.h"
#include <HighQueue/MemoryPoolFwd.h>
#include <Common/Log.h>

namespace HighQueue
{
    namespace Stages
    {
        /// @brief Base class for Stages
        /// 
        /// Stage life cycle:
        /// 1: Construct using Null constructor.
        /// 2: Configure    // capture parameters
        /// 3: Attach*      // attach destination(s) and resources as needed
        /// 4: Validate     // validate configuration and attachment
        /// 5: Start        // Acquire resources and go live
        ///                 // should not fail due to bad configuration or attachments. 
        ///                 // Problems should have been identified in initialize();
        ///                 // May still fail due to "runtime issues" (remote host denied connection, can't open file, etc.)
        /// 6: Handle       // Process incoming message.
        /// 7: Pause        // Temporary stop sending to destination (optional)
        /// 8: Resume       // Continue after a pause
        /// 9: Stop         // begin the shut-down process.  Stop producing output
        /// 10: Finish       // final cleanup 
        /// 11: Destruct     // should be nothing left, but check(?)

        class Stages_Export Stage: public std::enable_shared_from_this<Stage>
        {
        public:
            /// @brief Construct preferably with null constructor
            /// Lifecycle 1: Constuct
            Stage();

            /// @brief destruct
            /// Lifecycle 11: Destruct
            virtual ~Stage();

            /// @brief Configure 
            /// Lifecycle 2: Configure
            virtual bool configure(const ConfigurationNode & configuration);

            /// @brief Attach primary destination
            /// Lifecycle 3: Attach
            virtual void attachDestination(const StagePtr & destination);
            
            /// @brief Attach named destination
            /// Lifecycle 3: Attach
            virtual void attachDestination(const std::string & name, const StagePtr & destination);

            /// @brief Attach Connection
            virtual void attachConnection(const ConnectionPtr & connection);

            /// @brief Attach a memory pool to populate messages
            virtual void attachMemoryPool(const MemoryPoolPtr & pool);

            /// @brief Attach an ASIO IoService
            virtual void attachIoService(const AsioServicePtr & ioService);

            /// @brief Validate configuration and attachments
            /// Lifecycle 4: Validate
            virtual void validate();
            
            /// @brief Go Live
            /// Lifecycle 5: Start
            virtual void start();

            /// @brief Handle a message.
            /// Lifecycle 6: Handle
            /// @param message is the one to handle.
            /// @returns false if we should stop now.
            virtual void handle(Message & message) = 0;

            /// @brief Temporary stop sending to destination
            /// @lifecycle 7: Pause
            virtual void pause();

            /// @brief Resume after pause
            /// Lifecycle 8: Resume
            virtual void resume();

            /// @brief Stop
            /// Lifecycle 9: Stop
            virtual void stop();

            /// @brief Final cleanup
            /// Lifecycle 10: Finish
            virtual void finish();

            bool isStopping()const;
            bool isPaused() const;
        protected:
            void mustHaveDestination();
            void mustHaveDestination(const std::string & name);
            void mustNotHaveDestination();
            size_t destinationIndex(const std::string & name);
            void send(Message & message);
            void send(const std::string & name, Message & message);
            void send(size_t index, Message & message);

        protected:
            bool paused_;
            bool stopping_;
            StagePtr primaryDestination_;
            typedef std::pair<std::string,  StagePtr> NamedDestination;
            std::vector<NamedDestination> destinations_;
        };

        inline
        bool Stage::isStopping()const 
        { 
            return stopping_;
        }

        inline
        bool Stage::isPaused() const
        { 
            return paused_;
        }

        inline
        void Stage::send(Message & message)
        {
            primaryDestination_->handle(message);
            message.setEmpty();
        }

        inline
        void Stage::send(const std::string & name, Message & message)
        {
            for(auto & named : destinations_)
            {
                auto & destName = named.first;
                if(destName /*named.first */ == name)
                {
                    named.second->handle(message);
                    return;
                }
            }
            LogWarning("No destination named " << name << " was attached.");
        }

        inline
        void Stage::send(size_t index, Message & message)
        {
            destinations_[index].second->handle(message);
        }
   }
}