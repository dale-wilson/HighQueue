/// @file Connection.cpp
#include <Common/HighQueuePch.h>
#include "Producer.h"
#include <HighQueue/details/HQReservePosition.h>
using namespace HighQueue;

Producer::Producer(Connection & connection, bool solo)
: solo_(solo)
, connection_(connection)
, header_(connection.getHeader())
, entryCount_(header_->entryCount_)
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, reservePosition_(resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_)->reservePosition_)
, reserveSoloPosition_(resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_)->reserveSoloPosition_)
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, publishable_(0)
, statFulls_(0)
, statSkips_(0)
, statWaits_(0)
, statPublishes_(0)
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

void Producer::publish(Message & message)
{
    bool published = false;
    while(!published)
    {
        auto reserved = reserve();
        if(publishable_ <= reserved)
        {
            publishable_ = readPosition_ + entryCount_;
            while(publishable_ <= reserved)
            {
                ++statFulls_;
                std::this_thread::yield();
                std::atomic_thread_fence(std::memory_order::memory_order_consume);
                publishable_ = readPosition_ + header_->entryCount_;
            }
        }
        HighQEntry & entry = entryAccessor_[reserved];
        if(entry.status_ != HighQEntry::Status::SKIP)
        {
            message.moveTo(entry.message_);
            entry.status_ = HighQEntry::Status::OK;
            published = true;
        }
        else
        {
            ++statSkips_;
        }
        while(publishPosition_ < reserved)
        {
            ++statWaits_;
            std::this_thread::yield();
            std::atomic_thread_fence(std::memory_order::memory_order_acquire);
        }
        if(publishPosition_ == reserved)
        {
            ++publishPosition_;
            std::atomic_thread_fence(std::memory_order::memory_order_release);
            if(header_->consumerWaitStrategy_.mutexUsed_)
            {
                std::unique_lock<std::mutex> guard(header_->consumerWaitMutex_);
                header_->consumerWaitConditionVariable_.notify_all();
            }
            ++statPublishes_;
        }
    }
}

std::ostream & Producer::writeStats(std::ostream & out) const
{
    return out << "Full: " << statFulls_ 
               << " Skip: " << statSkips_
               << " Wait: " << statWaits_
               << " OK: " << statPublishes_
               << std::endl;

}


