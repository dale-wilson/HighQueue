/// @file Connection.cpp
#include <Common/MPassPch.h>
#include "Producer.h"
#include <ProntoQueue/details/PQReservePosition.h>
using namespace MPass;
using namespace ProntoQueue;

Producer::Producer(Connection & connection, bool solo)
: solo_(solo)
, connection_(connection)
, header_(connection.getHeader())
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, reservePosition_(resolver_.resolve<volatile PQReservePosition>(header_->reservePosition_)->reservePosition_)
, reserveSoloPosition_(resolver_.resolve<volatile PQReservePosition>(header_->reservePosition_)->reserveSoloPosition_)
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
{
    if(solo_ && reservePosition_ > reserveSoloPosition_)
    {
        reserveSoloPosition_ = reservePosition_;
    }
}

Producer::~Producer()
{
    if(solo_)
    {
        reservePosition_ = reserveSoloPosition_;
    }

}

inline
uint64_t Producer::reserve()
{
    if(solo_)
    {
        return reserveSoloPosition_++;
    }
    return reservePosition_++;
}

void Producer::publish(ProntoQueue::Message & message)
{
    bool published = false;
    while(!published)
    {
        auto reserved = reserve();
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




