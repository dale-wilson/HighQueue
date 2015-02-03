/// @file Consumer.cpp
#include <Common/HighQueuePch.h>
#include "Consumer.h"
#include <HighQueue/details/HQReservePosition.h>
using namespace HighQueue;

Consumer::Consumer(ConnectionPtr & connection)
: connection_(connection)
, header_(connection_->getHeader())
, producerUsesMutex_(header_->producerWaitStrategy_.mutexUsed_)
, resolver_(header_)
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, cachedPublishPosition_(publishPosition_)
, waitStrategy_(header_->consumerWaitStrategy_)
, spins_(waitStrategy_.spinCount_)
, yields_(waitStrategy_.yieldCount_)
, sleeps_(waitStrategy_.sleepCount_)
, stopping_(false)
, statGets_(0)
, statTrys_(0)
, statSpins_(0)
, statYields_(0)
, statSleeps_(0)
, statWaits_(0)
{
    if(header_->consumerPresent_.exchange(true))
    {
        throw std::runtime_error("Only one consumer can be attached to a HighQueue.");
    }
}

Consumer::~Consumer()
{
    header_->consumerPresent_ = false;
}

void Consumer::stop()
{
    stopping_ = true;
    header_->consumerWaitConditionVariable_.notify_all();
}

inline
void Consumer::incrementReadPosition()
{
    ++readPosition_;
#ifdef HIGHQUEUE_CPU_REQUIRES_RELEASE_FENCE
    std::atomic_thread_fence(std::memory_order::memory_order_release);
#endif //  HIGHQUEUE_CPU_REQUIRES_RELEASE_FENCE
    if(producerUsesMutex_)
    {
        std::unique_lock<std::mutex> guard(header_->waitMutex_);
        if(header_->producerWaiting_)
        {
            header_->producerWaiting_ = false;
            header_->producerWaitConditionVariable_.notify_all();
        }
    }
}

bool Consumer::tryGetNext(Message & message)
{
    ++statTrys_;
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
            incrementReadPosition();
            return true;
        }
        incrementReadPosition();
    }

}

std::ostream & Consumer::writeStats(std::ostream & out)const
{
    return out << "Get: " << statGets_ << " Try: " << statTrys_ << " Spin: " << statSpins_ << " Yield: " << statYields_ << " Sleep: " << statSleeps_ << " Wait: " << statWaits_ << std::endl;
}

bool Consumer::getNext(Message & message)
{
    ++statGets_;
    size_t remainingSpins = spins_;
    size_t remainingYields = yields_;
    size_t remainingSleeps = sleeps_;
    
    while(!stopping_)
    {
        if(tryGetNext(message))
        {
            return true;
        }
        if(remainingSpins > 0)
        {
            ++statSpins_;
            if(remainingSpins != WaitStrategy::FOREVER)
            {
                --remainingSpins;
            }
            std::atomic_thread_fence(std::memory_order::memory_order_consume);
        }
        else if(remainingYields > 0)
        {
            ++statYields_;
            if(remainingYields != WaitStrategy::FOREVER)
            {
                --remainingYields;
            }
            std::this_thread::yield();
        }
        else if(remainingSleeps > 0)
        {
            ++statSleeps_;
            if(remainingSleeps != WaitStrategy::FOREVER)
            {
                --remainingSleeps;
            }
            std::this_thread::sleep_for(waitStrategy_.sleepPeriod_);
        }
        else
        {
            ++statWaits_;
            std::unique_lock<std::mutex> guard(header_->waitMutex_);
            if(tryGetNext(message))
            {
                return true;
            }
            header_->consumerWaiting_ = true;
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
    return false;
}
    





