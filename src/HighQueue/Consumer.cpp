/// @file Consumer.cpp
#include <Common/HighQueuePch.h>
#include "Consumer.h"
#include <HighQueue/details/HQReservePosition.h>
using namespace HighQueue;

Consumer::Consumer(Connection & connection)
: connection_(connection)
, header_(connection.getHeader())
, resolver_(header_)
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, cachedPublishPosition_(publishPosition_)
, waitStrategy_(header_->consumerWaitStrategy_)
, spins_(waitStrategy_.spinCount_)
, yields_(waitStrategy_.yieldCount_)
, sleeps_(waitStrategy_.sleepCount_)
{
}

bool Consumer::tryGetNext(Message & message)
{
    while(true)
    {
        Position readPosition = readPosition_;
        if(readPosition >= cachedPublishPosition_)
        {
            std::atomic_thread_fence(std::memory_order::memory_order_consume);
            readPosition = readPosition_;
            cachedPublishPosition_ = publishPosition_;
            if(readPosition >= cachedPublishPosition_)
            {
                return false;
            }
        }
        HighQEntry & entry = entryAccessor_[readPosition];
        if(entry.status_ == HighQEntry::Status::OK)
        {
            entry.message_.moveTo(message);
            ++readPosition_;
            return true;
        }
        ++readPosition_;
    }

}

bool Consumer::getNext(Message & message)
{
    size_t remainingSpins = spins_;
    size_t remainingYields = yields_;
    size_t remainingSleeps = sleeps_;
    // todo: check for shutting down and return false
    while(true)
    {
        if(tryGetNext(message))
        {
            return true;
        }
        if(remainingSpins > 0)
        {
            if(remainingSpins != ConsumerWaitStrategy::FOREVER)
            {
                --remainingSpins;
            }
        }
        else if(remainingYields > 0)
        {
            if(remainingYields != ConsumerWaitStrategy::FOREVER)
            {
                --remainingYields;
            }
            std::this_thread::yield();
        }
        else if(remainingSleeps > 0)
        {
            if(remainingSleeps != ConsumerWaitStrategy::FOREVER)
            {
                --remainingSleeps;
            }
            std::this_thread::sleep_for(waitStrategy_.sleepPeriod_);
        }
        else
        {
            std::unique_lock<std::mutex> guard(header_->consumerWaitMutex_);
            if(tryGetNext(message))
            {
                return true;
            }
            if(header_->consumerWaitConditionVariable_.wait_for(guard, waitStrategy_.mutexWaitTimeout_)
                == std::cv_status::timeout)
            {
                if(tryGetNext(message))
                {
                    return true;
                }
                // todo: define a better exception
                throw std::runtime_error("Consumer wait timeout.");
            }
        }
    }
}
    




