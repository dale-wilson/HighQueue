// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <HighQueue/Producer.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        class ComponentBase: public std::enable_shared_from_this<ComponentBase>
        {
        public:
            ComponentBase();
            virtual ~ComponentBase();

            void start();
            void stop();
            void pause();
            void resume();
            virtual void run() = 0;
        protected:
            void startThread();
            virtual void doPause();
            virtual void doResume();
        protected:
            bool paused_;
            bool stopping_;
            std::shared_ptr<ComponentBase> me_;
            std::thread thread_;
        };

        inline
        ComponentBase::ComponentBase()
            : paused_(false)
            , stopping_(false)
        {
        }

        inline
        ComponentBase::~ComponentBase()
        {
            stop();
        }

        inline
        void ComponentBase::start()
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                &ComponentBase::startThread,
                this));
        }

        inline
        void ComponentBase::stop()
        {
            stopping_ = true;
            paused_ = false;
            if(me_)
            { 
                thread_.join();
                me_.reset();
            }
        }

        inline
        void ComponentBase::pause()
        {
            paused_ = true;
            doPause();
        }
        inline
        void ComponentBase::doPause()
        {
            // nothing here.  Intended to be overridden
        }

        inline
        void ComponentBase::resume()
        {
            paused_ = false;
            doResume();
        }
        inline
        void ComponentBase::doResume()
        {
            // nothing here.  Intended to be overridden.
        }


        inline
        void ComponentBase::startThread()
        {
            run();
        }
   }
}