#include <Common/HighQueuePch.h>
#include "Producer.h"
#include <HighQueue/details/HQReservePosition.h>

using namespace HighQueue;

namespace {
    const bool useSpinLock = true;
}

Producer::Producer(ConnectionPtr & connection)
: connection_(connection)
, solo_(connection_->canSolo())
, stopping_(false)
, header_(connection_->getHeader())
, entryCount_(header_->entryCount_)
, waitStrategy_(header_->producerWaitStrategy_)
, consumerUsesMutex_(header_->consumerWaitStrategy_.mutexUsed_)
, resolver_(header_)
, readPosition_(*resolver_.resolve<volatile Position>(header_->readPosition_))
, publishPosition_(*resolver_.resolve<volatile Position>(header_->publishPosition_))
, reserveStructure_(*resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_))
#if 0
, reservePosition_(resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_)->reservePosition_)
, reserveSoloPosition_(reinterpret_cast<volatile Position &>(resolver_.resolve<volatile HighQReservePosition>(header_->reservePosition_)->reservePosition_))
#else
, reservePosition_(reserveStructure_.reservePosition_)
, reserveSoloPosition_(reinterpret_cast<volatile Position &>(reservePosition_))
, reserveSpinLock_(const_cast<SpinLock &>(reserveStructure_.reserveSpinLock_))
#endif
, entryAccessor_(resolver_, header_->entries_, header_->entryCount_)
, publishable_(0)
, statFulls_(0)
, statSkips_(0)
, statPublishWaits_(0)
, statPublishInLine_(0)
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
Position Producer::reserve()
{
    if(solo_)
    {
        return reserveSoloPosition_++;
    }
    return reservePosition_++;
}

inline bool Producer::unreserve(Position reserve)
{
    if(solo_)
    {
        if(reserve < reserveSoloPosition_)
        {
            --reserveSoloPosition_;
        }
        return true;
    }
    else
    {
        if(reserve < reserveSoloPosition_)
        {
            Position expected = reserve + 1;
            return reservePosition_.compare_exchange_strong(expected, reserve);
        }
        return true;
    }

}

void Producer::publish(Message & message)
{
    bool published = false;
    while(!published)
    {
        SpinLock::Guard guard;
        if(useSpinLock && !solo_)
        {
            guard = SpinLock::Guard(reserveSpinLock_);
        }
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
                    if(stopping_ && unreserve(reserved))
                    {
                        return;
                    }

                    if(remainingSpins > 0)
                    {
                        spinDelay();
                        ++statSpins_;
                        if(remainingSpins != WaitStrategy::FOREVER)
                        {
                            spinDelay();
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
        // reserved now pints to a visible entry
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
        int64_t pending = reserved - publishPosition_;
        while(pending > 0)
        {
            /// todo simplify this now that we aren't using it!
            if(stopping_ && unreserve(reserved))
            {
                return;
            }

            ++statPublishWaits_;
            if(pending > 1)
            {
                ++statPublishInLine_;
                std::this_thread::yield();
            }
            else
            {
                std::atomic_thread_fence(std::memory_order::memory_order_acquire);
            }
            pending = reserved - publishPosition_;
        }
        // The following will *always* be true unless something is broken
        if(pending == 0)
        {
            ++publishPosition_;
            ++statPublishes_;
            if(!consumerUsesMutex_)
            {
                std::atomic_thread_fence(std::memory_order::memory_order_release);
                return;
            }

            std::unique_lock<std::mutex> guard(header_->waitMutex_);
            if(header_->consumerWaiting_)
            {
                header_->consumerWaiting_ = false;
                header_->consumerWaitConditionVariable_.notify_all();
            }
        }
    }
}

void Producer::stop()
{
    stopping_ = true;
    std::unique_lock<std::mutex> guard(header_->waitMutex_);
    header_->producerWaitConditionVariable_.notify_all();
}

std::ostream & Producer::writeStats(std::ostream & out) const
{
    return out << "Published " << statPublishes_
               << " Full: " << statFulls_
               << " Skip: " << statSkips_
               << " WaitOtherPublishers: " << statPublishWaits_
               << "/" << statPublishInLine_
               << " Spin: " << statSpins_ 
               << " Yield: " << statYields_ 
               << " Sleep: " << statSleeps_ 
               << " Wait: " << statWaits_
               << " OK: " << statPublishes_
               << std::endl;

}


