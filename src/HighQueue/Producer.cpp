#include <Common/HighQueuePch.h>
#include "Producer.h"
#include <HighQueue/details/HQReservePosition.h>
using namespace HighQueue;

Producer::Producer(ConnectionPtr & connection, bool solo)
: solo_(solo)
, connection_(connection)
, header_(connection_->getHeader())
, entryCount_(header_->entryCount_)
, waitStrategy_(header_->producerWaitStrategy_)
, consumerUsesMutex_(header_->consumerWaitStrategy_.mutexUsed_)
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, reservePosition_(resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_)->reservePosition_)
, reserveSoloPosition_(reinterpret_cast<volatile Position &>(resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_)->reservePosition_))
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, publishable_(0)
, statFulls_(0)
, statSkips_(0)
, statPublishWaits_(0)
, statPublishes_(0)
, statSpins_(0)
, statYields_(0)
, statSleeps_(0)
, statWaits_(0)
{
    ++header_->producersPresent_;
}

Producer::~Producer()
{
    --header_->producersPresent_;
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
            if(publishable_ <= reserved)
            {
                ++statFulls_;
                if(header_->discardMessagesIfNoConsumer_ && !header_->consumerPresent_)
                {
                    readPosition_ = publishPosition_;
                    publishable_ = readPosition_ + entryCount_;
                }
                size_t remainingSpins = waitStrategy_.spinCount_;
                size_t remainingYields = waitStrategy_.yieldCount_;
                size_t remainingSleeps = waitStrategy_.sleepCount_;
                while(publishable_ <= reserved)
                {
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
                        publishable_ = readPosition_ + entryCount_;
                        if(publishable_ > reserved)
                        {
                            header_->producerWaiting_ = true;
                            if(header_->producerWaitConditionVariable_.wait_for(guard, waitStrategy_.mutexWaitTimeout_)
                                == std::cv_status::timeout)
                            {
                                publishable_ = readPosition_ + entryCount_;
                                if(publishable_ <= reserved)
                                {
                                    // todo: define a better exception
                                    throw std::runtime_error("Producer wait timeout.");
                                }
                            }
                        }
                    }
                    publishable_ = readPosition_ + header_->entryCount_;
                }

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
            ++statPublishWaits_;
            std::this_thread::yield();
            std::atomic_thread_fence(std::memory_order::memory_order_acquire);
        }
        if(publishPosition_ == reserved)
        {
            ++publishPosition_;
            std::atomic_thread_fence(std::memory_order::memory_order_release);
            if(consumerUsesMutex_)
            {
                std::unique_lock<std::mutex> guard(header_->waitMutex_);
                if(header_->consumerWaiting_)
                {
                    header_->consumerWaiting_ = false;
                    header_->consumerWaitConditionVariable_.notify_all();
                }
            }
            ++statPublishes_;
        }
    }
}

std::ostream & Producer::writeStats(std::ostream & out) const
{
    return out << "Published " << statPublishes_
               << " Full: " << statFulls_
               << " Skip: " << statSkips_
               << " WaitOtherPublishers: " << statPublishWaits_
               << " Spin: " << statSpins_ 
               << " Yield: " << statYields_ 
               << " Sleep: " << statSleeps_ 
               << " Wait: " << statWaits_
               << " OK: " << statPublishes_
               << std::endl;

}


