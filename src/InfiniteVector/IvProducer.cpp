/// @file IvConnection.cpp
#include <Common/MPassPch.h>
#include "IvProducer.h"
#include <InfiniteVector/IvReservePosition.h>
using namespace MPass;
using namespace InfiniteVector;

IvProducer::IvProducer(IvConnection & connection)
: connection_(connection)
, header_(connection.getHeader())
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, reservePosition_(resolver_.resolve<volatile IvReservePosition>(header_->reservePosition_)->reservePosition_)
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
{
}

void IvProducer::publish(InfiniteVector::Message & message)
{
    bool published = false;
    while(!published)
    {
        auto reserved = reservePosition_++;
        auto entryEnd = readPosition_ + header_->entryCount_;
        while(entryEnd <= reserved)
        {
            std::_Atomic_thread_fence(std::memory_order::memory_order_consume);
            entryEnd = readPosition_ + header_->entryCount_;
        }
        IvEntry & entry = entryAccessor_[reserved];
        if(entry.status_ != IvEntry::Status::SKIP)
        {
            message.moveTo(entry.message_);
            entry.status_ = IvEntry::Status::OK;
            published = true;
        }
        while(publishPosition_ < reserved)
        {
            std::this_thread::yield();
            std::atomic_thread_fence(std::memory_order::memory_order_acquire);
        }
        if(publishPosition_ == reserved)
        {
            ++publishPosition_;
            if(header_->consumerWaitStrategy_.mutexUsed_)
            {
                std::unique_lock<std::mutex> guard(header_->consumerWaitMutex_);
                header_->consumerWaitConditionVariable_.notify_all();
            }
        }
        std::atomic_thread_fence(std::memory_order::memory_order_release);
    }
}




