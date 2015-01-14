// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <ComponentCommon/HeaderGenerator.h>
#include <HighQueue/Consumer.h>

#define USE_DEBUG_MESSAGE 0
#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<typename CargoType>
        class PassThru : public std::enable_shared_from_this<PassThru<CargoType> >
        {
        public:
            enum CopyType
            {
                CopyBinary,
                CopyConstruct,
                CopyMove,
                CopyForward
            };
            PassThru(ConnectionPtr & inConnection, ConnectionPtr & outConnection, CopyType copyType, uint32_t messageCount = 0, bool quitOnEmptyMessage = true);
            ~PassThru();

            void start();
            void stop();
            void pause();
            void resume();

            void run();
        private:
            ConnectionPtr inConnection_;
            ConnectionPtr outConnection_;
            Consumer consumer_;
            Producer producer_;
            Message inputMessage_;
            Message outputMessage_;
            CopyType copyType_;
            uint32_t messageCount_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;

            std::shared_ptr<PassThru> me_;
            std::thread thread_;
        };

        template<typename CargoType>
        PassThru<CargoType>::PassThru(ConnectionPtr & inConnection, ConnectionPtr & outConnection, CopyType copyType, uint32_t messageCount, bool quitOnEmptyMessage)
            : inConnection_(inConnection)
            , outConnection_(outConnection)
            , consumer_(inConnection_)
            , producer_(outConnection_)
            , inputMessage_(inConnection)
            , outputMessage_(outConnection)
            , copyType_(copyType)
            , messageCount_(messageCount)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , paused_(false)
            , stopping_(false)
        {
        }

        template<typename CargoType>
        PassThru<CargoType>::~PassThru()
        {
            stop();
        }

        template<typename CargoType>
        void PassThru<CargoType>::start()
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                &PassThru<CargoType>::run,
                this));
        }

        template<typename CargoType>
        void PassThru<CargoType>::stop()
        {
            stopping_ = true;
            if(me_)
            { 
                thread_.join();
                me_.reset();
            }
        }

        template<typename CargoType>
        void PassThru<CargoType>::pause()
        {
            paused_ = true;
        }

        template<typename CargoType>
        void PassThru<CargoType>::resume()
        {
            paused_ = false;
        }

        template<typename CargoType>
        void PassThru<CargoType>::run()
        {
            DebugMessage("PassThru  start. " << inConnection_->getHeader()->name_ << "->" << outConnection_->getHeader()->name_ << std::endl);
            uint32_t messageCount = 0; 
            while(!stopping_)
            {
                stopping_ = !consumer_.getNext(inputMessage_);
                size_t used = inputMessage_.getUsed();

                if(!stopping_ && quitOnEmptyMessage_ && inputMessage_.isEmpty())
                {
                    producer_.publish(outputMessage_);
                    stopping_ = true;                   
                }
                if(!stopping_)
                { 
                    switch(copyType_)
                    {
                        default:
                        case CopyBinary:
                        {
                            DebugMessage("PassThru  copy binary.\n");
                            outputMessage_.appendBinaryCopy(inputMessage_.get(), inputMessage_.getUsed());
                            producer_.publish(outputMessage_);
                            break;
                        }
                        case CopyConstruct:
                        {
                            DebugMessage("PassThru  copy construct.\n");
                            outputMessage_.appendEmplace<CargoType>(*inputMessage_.get<CargoType>());
                            producer_.publish(outputMessage_);
                            break;
                        }
                        case CopyMove:
                        {
                            DebugMessage("PassThru  copy move.\n");
                            inputMessage_.moveTo(outputMessage_);
                            producer_.publish(outputMessage_);
                            break;
                        }
                        case CopyForward:
                        {
                            DebugMessage("PassThru  copy Forward.\n");
                            producer_.publish(inputMessage_);
                            break;
                        }
                    }
                }
            }
            DebugMessage("Pass thru " << messageCount << " messages" << std::endl);
        }
   }
}