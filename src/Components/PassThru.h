// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once
#include <HighQueue/Consumer.h>

#include <ComponentCommon/DebugMessage.h>

namespace HighQueue
{
    namespace Components
    {
        template<typename CargoMessage>
        class PassThru : public std::enable_shared_from_this<PassThru<CargoMessage> >
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

            static const char * copyTypeName(CopyType type)
            {
                switch(type)
                {
                    case CopyBinary:
                        return "Binary";
                    case CopyConstruct:
                        return "Construct";
                    case CopyMove:
                        return "Move";
                    case CopyForward:
                        return "Forward";
                    default:
                        return "Invalid";
                }
            }
        private:
            ConnectionPtr inConnection_;
            ConnectionPtr outConnection_;
            Consumer consumer_;
            Producer producer_;
            Message message_;
            Message outputMessage_;
            CopyType copyType_;
            uint32_t messageCount_;
            bool quitOnEmptyMessage_;
            bool paused_;
            bool stopping_;

            std::shared_ptr<PassThru> me_;
            std::thread thread_;
        };

        template<typename CargoMessage>
        PassThru<CargoMessage>::PassThru(ConnectionPtr & inConnection, ConnectionPtr & outConnection, CopyType copyType, uint32_t messageCount, bool quitOnEmptyMessage)
            : inConnection_(inConnection)
            , outConnection_(outConnection)
            , consumer_(inConnection_)
            , producer_(outConnection_)
            , message_(inConnection)
            , outputMessage_(outConnection)
            , copyType_(copyType)
            , messageCount_(messageCount)
            , quitOnEmptyMessage_(quitOnEmptyMessage)
            , paused_(false)
            , stopping_(false)
        {
        }

        template<typename CargoMessage>
        PassThru<CargoMessage>::~PassThru()
        {
            stop();
        }

        template<typename CargoMessage>
        void PassThru<CargoMessage>::start()
        {
            me_ = shared_from_this();
            thread_ = std::thread(std::bind(
                &PassThru<CargoMessage>::run,
                this));
        }

        template<typename CargoMessage>
        void PassThru<CargoMessage>::stop()
        {
            stopping_ = true;
            if(me_)
            { 
                thread_.join();
                me_.reset();
            }
        }

        template<typename CargoMessage>
        void PassThru<CargoMessage>::pause()
        {
            paused_ = true;
        }

        template<typename CargoMessage>
        void PassThru<CargoMessage>::resume()
        {
            paused_ = false;
        }

        template<typename CargoMessage>
        void PassThru<CargoMessage>::run()
        {
            DebugMessage("PassThru  start. " << inConnection_->getHeader()->name_ << "->" << outConnection_->getHeader()->name_ << std::endl);
            uint32_t messageCount = 0; 
            while(!stopping_)
            {
                stopping_ = !consumer_.getNext(message_);
                size_t used = message_.getUsed();

                if(!stopping_ && quitOnEmptyMessage_ && message_.isEmpty())
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
                            outputMessage_.appendBinaryCopy(message_.get(), message_.getUsed());
                            outputMessage_.meta() = message_.meta();
                            producer_.publish(outputMessage_);
                            break;
                        }
                        case CopyConstruct:
                        {
                            DebugMessage("PassThru  copy construct.\n");
                            outputMessage_.emplace<CargoMessage>(*message_.get<CargoMessage>());
                            outputMessage_.meta() = message_.meta();
                            producer_.publish(outputMessage_);
                            break;
                        }
                        case CopyMove:
                        {
                            DebugMessage("PassThru  copy move.\n");
                            message_.moveTo(outputMessage_);
                            producer_.publish(outputMessage_);
                            break;
                        }
                        case CopyForward:
                        {
                            DebugMessage("PassThru  copy Forward.\n");
                            producer_.publish(message_);
                            break;
                        }
                    }
                }
            }
            DebugMessage("Pass thru " << messageCount << " messages" << std::endl);
        }
   }
}