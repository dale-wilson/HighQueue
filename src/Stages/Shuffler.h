// Copyright (c) 2015 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include "ShufflerFwd.h"

#include <StagesSupport/StageToMessage.h>

namespace HighQueue
{
    namespace Stages
    {
        class Shuffler : public StageToMessage
        {
        public:
			static const size_t relativelyPrime_ = 101;
            Shuffler(size_t lookAhead);

			virtual void attachConnection(const ConnectionPtr & connection);
            virtual void attachMemoryPool(const MemoryPoolPtr & memoryPool);

            virtual void validate();
            virtual void handle(Message & message);

        private:
            void publishPendingMessages();

            void handleHeartbeat(Message & message);
            void handleShutdown(Message & message);
            void handleDataMessage(Message & message);

        private:
            size_t lookAhead_;
            std::vector<Message> pendingMessages_;
			uint64_t position_;
        };

        inline
        Shuffler::Shuffler(size_t lookAhead)
            : lookAhead_(lookAhead)
			, position_(0)
        {
            setName("Shuffler"); // default name
        }

        inline
        void Shuffler::attachConnection(const ConnectionPtr & connection)
        {      
            while(pendingMessages_.size() < lookAhead_)
            {
                pendingMessages_.emplace_back(connection);
            }
            StageToMessage::attachConnection(connection);
        }

        inline
        void Shuffler::attachMemoryPool(const MemoryPoolPtr & memoryPool)
        {
            while(pendingMessages_.size() < lookAhead_)
            {
                pendingMessages_.emplace_back(memoryPool);
            }
            StageToMessage::attachMemoryPool(memoryPool);
        }

        inline
        void Shuffler::validate()
        {
            if(pendingMessages_.size() < lookAhead_)
            {
                throw std::runtime_error("Shuffler working messages not initialized. Missing call to attachConnection or attachMemoryPool?");
            }
            StageToMessage::validate();
        }

        inline
        void Shuffler::handle(Message & message)
        {
            auto type = message.getType();
            if(type == Message::MessageType::Heartbeat)
            {
                handleHeartbeat(message);
            }
            else if(type == Message::MessageType::Shutdown)
            {
                handleShutdown(message);
            }
            else
            {
                handleDataMessage(message);
            }
        }

        inline
        void Shuffler::handleHeartbeat(Message & message)
        {
			publishPendingMessages();
		}

        inline
        void Shuffler::handleShutdown(Message & message)
        {
			// make the user call stop();
			handleDataMessage(message);
			publishPendingMessages();
		}

        inline
        void Shuffler::handleDataMessage(Message & message)
        {
			position_ += relativelyPrime_;
			size_t index = position_ % lookAhead_;
			if(!pendingMessages_[index].isEmpty())
			{
				send(pendingMessages_[index]);
			}
			message.moveTo(pendingMessages_[index]);
        }

        inline
        void Shuffler::publishPendingMessages()
        {
			for(auto & message : pendingMessages_)
			{
				if(!message.isEmpty())
				{
					send(message);
				}
			}
		}
   }
}