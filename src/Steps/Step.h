// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "StepFwd.h"
#include <Steps/Step_Export.h>
#include <Steps/SharedResourcesFwd.h>
#include <Steps/ConfigurationFwd.h>
#include "HighQueue/ConnectionFwd.h"
#include <HighQueue/MemoryPoolFwd.h>
#include "HighQueue/MessageFwd.h"

#include <Common/Log.h>

namespace HighQueue
{
    namespace Steps
    {
        /// @brief Base class for Steps
        /// 
        /// Step life cycle:
        /// 0: Register.
        ///    A Step class should register itself with the StepFactory by creating a static instance of 
        ///    StepFactory::Registrar<stepclass>.
        ///
        /// 1: Construct
        ///    A Step should have a null constructor so it can be created with a StepFactory.
        ///
        /// 2: Configure
        ///   2a: Read configuration parameters.
        ///       The step gets configuration parameters(if any) from configurationNode. 
        ///   2b: Configure resources
        ///       Sets parameters for shared resources in BulderResource.  Shared resources include a
        ///       memory pool, an AsioService, etc.
        ///
        /// 3 Attachment
        ///   3a: Attach Destination(s)
        ///       Zero or more destinations may be attached. Messages generated by the step should be sent to
        ///       these destinations.
        ///   3b: Attach Resources 
        ///       The resources configured during [2: Configure] will have been created.  
        ///       The Step can capture the ones it needs.
        ///
        /// 4: Validate
        ///    Each Step checks to be sure all required parameters are configured and make sense, and that
        ///    all necessary attachments are available.  If errors are detected, the Step should throw an 
        ///    exception.
        ///    If all Steps pass validation then the system will go live.
        ///
        /// 5: Start
        ///    The system is going live.  The Step should Open files, sockets, place initial ASIO requests,etc.
        ///    After this call the step may generate messages for following steps an may receive messages from
        ///    previous steps.
        ///
        ///    Start should not fail due to missing configuration or attachments, because problems should have
        ///    been identified during Validate; however Start may still fail due to runtime issues (remote host 
        ///    denied connection, can't open file, etc.)  If it fails, the Step will throw an exception and the
        ///    system will immediately shut down again.
        ///
        /// 6: Handle
        ///    Process an incoming message.
        ///           
        /// 7: Pause
        ///    Temporarily stop sending to destination (optional)
        ///
        /// 8: Resume
        ///    Continue after a pause
        ///
        /// 9: Stop
        ///   Begin the shut-down process.  Stop producing output messages for following steps.
        ///   Note the step may still receive input messages from previous steps for a brief period of time.
        ///
        /// 10: Finish
        ///   No Messages from previous steps will arive.  The step should join threads, close files, etc. as 
        ///   necessary to allow a clean shut down of the system.
        ///
        /// 11: Destruct
        ///     There should be nothign left to do at this point, but check anyway if it matters.

        class Steps_Export Step: public std::enable_shared_from_this<Step>
        {
        public:
            static const std::string keyName;
            /// @brief Construct 
            /// Lifecycle 1: Constuct
            Step();

            /// @brief destruct
            /// Lifecycle 11: Destruct
            virtual ~Step();

            /// @brief Configure 
            /// Lifecycle 2: Configure
            virtual bool configure(const ConfigurationNode & configuration);
            
            /// @brief Configure a single parameter
            /// Called from the default implementation of configure();
            /// @param key the key to identifiy the parameter.
            /// @param configuration the node containing the value of the parameter.
            virtual bool configureParameter(const std::string & key, const ConfigurationNode & configuration);

            /// @brief Write a "usage" message to out.
            /// Will be called if configure fails, so it should mention any configuration parameters
            /// expected by configure.
            virtual std::ostream & usage(std::ostream & out) const;

            /// @brief Prepare shared resources based on information gleanded from configure.
            /// @param resources has "need*" methods which this step can used to indicated what resources it will use.
            virtual void configureResources(SharedResources & resources);

            /// @brief Configure
            /// Lifecycle 2a: Give it a name
            void setName(const std::string & name);

            /// @brief Attach primary destination
            /// Lifecycle 3: Attach
            virtual void attachDestination(const StepPtr & destination);
            
            /// @brief Attach named destination
            /// Lifecycle 3: Attach
            virtual void attachDestination(const std::string & name, const StepPtr & destination);

            /// @brief Attach resources
            virtual void attachResources(SharedResources & resources);

            /// @brief Validate configuration and attachments
            /// Lifecycle 4: Validate
            virtual void validate();
            
            /// @brief Go Live
            /// Lifecycle 5: Start
            virtual void start();

            /// @brief Handle a message.
            /// Lifecycle 6: Handle
            /// Default behavior is to throw a runtime_error exception.
            /// Override if this step accepts incoming messages.
            /// @param message is the one to handle.
            /// @returns false if we should stop now.
            virtual void handle(Message & message);

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
            
            const std::string & getName()const;
            bool isStopping()const;
            bool isPaused() const;

            typedef std::function<bool(const StepPtr &, const std::string &, const ConfigurationNode &)> ParameterHandler;
            void setParameterHandler(ParameterHandler handler);
        protected:
            void mustHaveDestination();
            void mustHaveDestination(const std::string & name);
            void mustNotHaveDestination();
            size_t destinationIndex(const std::string & name);
            size_t getDestinationCount()const;
            void send(Message & message);
            void send(const std::string & name, Message & message);
            void send(size_t index, Message & message);

        protected:
            bool paused_;
            bool stopping_;
            std::string name_;
            StepPtr primaryDestination_;
            typedef std::pair<std::string,  StepPtr> NamedDestination;
            std::vector<NamedDestination> destinations_;
            ParameterHandler parameterHandler_;
        };

        inline
        bool Step::isStopping()const 
        { 
            return stopping_;
        }

        inline
        bool Step::isPaused() const
        { 
            return paused_;
        }

        inline
        void Step::send(Message & message)
        {
            primaryDestination_->handle(message);
            message.setEmpty();
        }

        inline
        void Step::send(const std::string & name, Message & message)
        {
            for(auto & named : destinations_)
            {
                auto & destName = named.first;
                if(destName /*named.first */ == name)
                {
                    named.second->handle(message);
                    message.setEmpty();
                    return;
                }
            }
            LogWarning("No destination named " << name << " was attached.");
        }

        inline
        void Step::send(size_t index, Message & message)
        {
            destinations_[index].second->handle(message);
            message.setEmpty();
        }
   }
}