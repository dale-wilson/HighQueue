/// @file IvConnection.cpp
#include <Common/MPassPch.h>
#include "IvConsumer.h"
#include <InfiniteVector/IvReservePosition.h>
using namespace MPass;
using namespace InfiniteVector;

IvConsumer::IvConsumer(IvConnection & connection)
: connection_(connection)
, header_(connection.getHeader())
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, publishedPosition_(publishPosition_)
{
}

bool IvConsumer::tryGetNext(Buffers::Buffer & buffer)
{
    while(true)
    {
        Position readPosition = readPosition_;
        if(readPosition >= publishedPosition_)
        {
            std::_Atomic_thread_fence(std::memory_order::memory_order_consume);
            readPosition = readPosition_;
            publishedPosition_ = publishPosition_;
            if(readPosition >= publishedPosition_)
            {
                return false;
            }
        }
        IvEntry & entry = entryAccessor_[readPosition];
        if(entry.status_ == IvEntry::Status::OK)
        {
            entry.buffer_.moveTo(buffer);
            ++readPosition_;
            return true;
        }
        ++readPosition_;
    }

}

void IvConsumer::getNext(Buffers::Buffer & buffer)
{
    while(true)
    {
        if(tryGetNext(buffer))
        {
            return;
        }
        int todo_consumer_wait_strategy;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
    





